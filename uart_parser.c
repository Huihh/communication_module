

#include "uart_parser.h"
#include "uart_method.h"
#include "uart_constants.h"
#include "uart_transfer_api.h"
#include "uart_log.h"



static int32_t uart_cmd_process(void *handle, uint8_t func_code, uint8_t *data, uint16_t data_len)
{
	uint8_t ret_timer_code, idle_index;
	int32_t ret_code;

	if ( (data_len + PACKET_EXT_INFO) > UART_TX_BUF_MAX_LEN )
	{
		LOG_UART_DEBUG("(%s)-->Send buf overflow\n", __FUNCTION__);
		return -ERR_SEND_BUF_OVERFLOW;
	}

	LOG_UART_DEBUG("Send Cmd... data_len = %d\n", data_len);

	t_class_uart_parser *ptr_handle = (t_class_uart_parser *)handle;

	if ( (ptr_handle->pri_mem.parser_init_flag == MODULE_UNINT) || (ptr_handle->pri_mem.timer_init_flag == MODULE_UNINT) )
	{
		LOG_UART_DEBUG("(%s)-->Modlue not init\n", __FUNCTION__);
		return -ERR_CALLBACK_UNINIT;
	}	

	idle_index = get_idle_index(&(ptr_handle->pri_mem.data_queue), ENUM_SEND_QUEUE);

	if ( (true == data_queue_is_full(&(ptr_handle->pri_mem.data_queue), ENUM_SEND_QUEUE)) || (idle_index == 0xFF) )
	{
		LOG_UART_DEBUG("(%s)-->data queue is full, send_buf_num = %d, idle_index = %d\n", __FUNCTION__, ptr_handle->pri_mem.data_queue.send_buf_num, idle_index);
		return -ERR_QUEUE_FULL;
	}

	if ( (func_code != FUNC_CODE_READ) && (func_code != FUNC_CODE_WRITE) && (func_code != FUNC_CODE_REPORT) ) 
	{
		LOG_UART_DEBUG("(%s)-->func_code is illegal\n", __FUNCTION__);
		return -ERR_FUNC_CODE_INVALID;
	}

	LOG_UART_DEBUG("Reassembly packet (seq_num = %d)...\n", ptr_handle->pri_mem.send_seq_num);

	ptr_handle->pri_mem.send_seq_num = (ptr_handle->pri_mem.send_seq_num == 0xFF) ? 0x01 : (ptr_handle->pri_mem.send_seq_num + 0x01);

	ptr_handle->pri_mem.send_len = reassembly_packet(ptr_handle->pri_mem.send_seq_num, func_code, ptr_handle->pri_mem.data_queue.send_buf[idle_index], data, data_len);

	LOG_UART_DEBUG("Send data (len = %d)...\n", ptr_handle->pri_mem.send_len);
	LOG_UART_ARRAY_DEBUG(ptr_handle->pri_mem.data_queue.send_buf[idle_index], ptr_handle->pri_mem.send_len);


	ret_code = ptr_handle->func_send_cb(ptr_handle, ptr_handle->pri_mem.data_queue.send_buf[idle_index], ptr_handle->pri_mem.send_len);
	if (ret_code != RET_CODE_SUCESS)
	{
		LOG_UART_DEBUG("(%s)-->Send ret_code = %d\n", __FUNCTION__, ret_code);
		return -ERR_UART_SEND_ERR;
	}

	ptr_handle->pri_mem.data_queue.send_retry_times[idle_index] = RETRY_MAX_TIMES;
	ptr_handle->pri_mem.data_queue.send_waitting_rsp[idle_index] = 0x01;
	ptr_handle->pri_mem.data_queue.send_time_expired_stamp[idle_index] = TIMER_TIMEOUT_SLOT;

	ptr_handle->pri_mem.data_queue.send_idle_index[idle_index] = ptr_handle->pri_mem.send_seq_num;
 
	ptr_handle->pri_mem.data_queue.send_buf_num += 0x01;

	if (ptr_handle->pri_mem.data_queue.send_buf_num == QUEUE_SEND_SIZE)
	{
		LOG_UART_DEBUG("Send buf is full, cannot fill any cmd. send_cmd_num = %d, queue_deep = %d\n", ptr_handle->pri_mem.data_queue.send_buf_num, QUEUE_SEND_SIZE);
	}

	LOG_UART_DEBUG("Start timer...\n");

	ret_timer_code = ptr_handle->timer_start_cb(ptr_handle->p_timer_id, TIMER_TIMEOUT_MS, ptr_handle);
	if (ret_timer_code != RET_CODE_SUCESS)
	{
		LOG_UART_DEBUG("(%s)-->Start timer Failed...\n", __FUNCTION__);

		if (ptr_handle->pri_mem.parser_init_flag == MODULE_INIT)
		{
			ptr_handle->func_event_cb(ptr_handle, EVT_TIMER_FAILED, ptr_handle->pri_mem.send_seq_num, NULL, (uint16_t *)&(ptr_handle->pri_mem.send_len));
		}
	}

	LOG_UART_DEBUG("Reassembly packet send_seq_num = %d\n", ptr_handle->pri_mem.send_seq_num);
	return ptr_handle->pri_mem.send_seq_num;
}




static int32_t uart_recv_process(void *handle, uint8_t *data, uint16_t data_len)
{
	uint8_t ret_timer_code, idle_index;
	uint32_t packet_len;
	bool ret_result;

	LOG_UART_DEBUG("Recv data... data_len = %d\n", data_len);

	t_class_uart_parser *ptr_handle = (t_class_uart_parser *)handle;

	if ( (ptr_handle->pri_mem.parser_init_flag == MODULE_UNINT) || (ptr_handle->pri_mem.timer_init_flag == MODULE_UNINT) )
	{
		LOG_UART_DEBUG("(%s)-->Modlue not init\n", __FUNCTION__);
		return -ERR_CALLBACK_UNINIT;
	}	


	if (ptr_handle->pri_mem.recv_len == 0x00)
	{
		ret_result = search_start_code(data, (uint16_t *)&data_len);
		if (ret_result == false)
		{
			LOG_UART_DEBUG("(%s)-->Not find start code\n", __FUNCTION__);
			return -ERR_NO_START_CODE;
		}
	}
	else
	{
		if ( (ptr_handle->pri_mem.recv_len + data_len) > CACHE_BUF_MAX_LEN )
		{
			LOG_UART_DEBUG("(%s)-->Cache buf is overflow, discard cache buf data\n", __FUNCTION__);
			ptr_handle->pri_mem.recv_len = 0x00;
			return -ERR_CAHCE_BUF_OVERFLOW;
		} 	
	}
	
	memmove(&(ptr_handle->pri_mem.cache_buf[ptr_handle->pri_mem.recv_len]), data, data_len);

	ptr_handle->pri_mem.recv_len += data_len;

	LOG_UART_ARRAY_DEBUG(ptr_handle->pri_mem.cache_buf, ptr_handle->pri_mem.recv_len);


	while (ptr_handle->pri_mem.recv_len != 0x00)
	{
		LOG_UART_DEBUG("cache buf data_len = %d\n", ptr_handle->pri_mem.recv_len);
		LOG_UART_ARRAY_DEBUG(ptr_handle->pri_mem.cache_buf, ptr_handle->pri_mem.recv_len);

		ret_result = search_start_code(&(ptr_handle->pri_mem.cache_buf[0]), (uint16_t *)&(ptr_handle->pri_mem.recv_len));
		if (ret_result == false)
		{
			LOG_UART_DEBUG("(%s)-->Not find start code, recv_len = %d\n", __FUNCTION__, ptr_handle->pri_mem.recv_len);
			continue;
		}

		if (ptr_handle->pri_mem.recv_len <= CHECK_SUM_OFF)
		{
			LOG_UART_DEBUG("Cache buf data less than or equals CHECK_SUM_OFF=%d\n", CHECK_SUM_OFF);
			break;
		}
		else
		{

			LOG_UART_DEBUG("verify checksum...\n");
			ret_result = verify_checksum(&(ptr_handle->pri_mem.cache_buf[0]), ptr_handle->pri_mem.recv_len);
			if (ret_result == false)
			{
				LOG_UART_DEBUG("(%s)-->Checksum failed\n", __FUNCTION__);
				
				ptr_handle->pri_mem.recv_len -= DATA_OFF;
				memmove(&(ptr_handle->pri_mem.cache_buf[0]), &(ptr_handle->pri_mem.cache_buf[DATA_OFF]), ptr_handle->pri_mem.recv_len);
				continue;
			}

			LOG_UART_DEBUG("verify checksum passed\n");

			packet_len = (((ptr_handle->pri_mem.cache_buf[DATA_LEN_OFF]) << 8) | (ptr_handle->pri_mem.cache_buf[DATA_LEN_OFF + 1])) + PACKET_EXT_INFO;

			if (packet_len > UART_RX_BUF_MAX_LEN)
			{
				LOG_UART_DEBUG("(%s)-->rx buf can not fit this packet, discard cache buf data, packet_len = %d, rx_buf_max_len = %d\n", __FUNCTION__, packet_len, UART_RX_BUF_MAX_LEN);
				ptr_handle->pri_mem.recv_len = 0x00;
				continue;
			}	


			if ( (ptr_handle->pri_mem.recv_len) < packet_len )
			{
				LOG_UART_DEBUG("(%s)-->Cache buf data less than complete packet, packet_len = %d, rx_buf_max_len = %d\n", __FUNCTION__, packet_len, UART_RX_BUF_MAX_LEN);
				break;
			}

			LOG_UART_DEBUG("verify crc16...\n");
			ret_result = verify_crc16(&(ptr_handle->pri_mem.cache_buf[0]), ptr_handle->pri_mem.recv_len);
			if (ret_result == false)
			{
				LOG_UART_DEBUG("(%s)-->CRC16 failed\n", __FUNCTION__);

				ptr_handle->pri_mem.recv_len -= packet_len;
				memmove(&(ptr_handle->pri_mem.cache_buf[0]), &(ptr_handle->pri_mem.cache_buf[packet_len]), ptr_handle->pri_mem.recv_len);
				continue;
			}

			LOG_UART_DEBUG("verify CRC16 passed\n");


			idle_index = get_idle_index(&(ptr_handle->pri_mem.data_queue), ENUM_RECV_QUEUE);

			if ( (true == data_queue_is_full(&(ptr_handle->pri_mem.data_queue), ENUM_RECV_QUEUE)) || (idle_index == 0xFF) )
			{
				LOG_UART_DEBUG("(%s)-->data queue is full, keep cache buf data, recv_len = %d\n", __FUNCTION__, ptr_handle->pri_mem.recv_len);
				break;
			}

			ptr_handle->pri_mem.recv_len -= packet_len;
			memmove(ptr_handle->pri_mem.data_queue.recv_buf[idle_index], &(ptr_handle->pri_mem.cache_buf[0]), packet_len);
			memmove(&(ptr_handle->pri_mem.cache_buf[0]), &(ptr_handle->pri_mem.cache_buf[packet_len]), ptr_handle->pri_mem.recv_len);

			LOG_UART_DEBUG("After split buf data_len = %d\n", ptr_handle->pri_mem.recv_len);
			LOG_UART_ARRAY_DEBUG(ptr_handle->pri_mem.cache_buf, ptr_handle->pri_mem.recv_len);

			ptr_handle->pri_mem.data_queue.recv_idle_index[idle_index] = ptr_handle->pri_mem.data_queue.recv_buf[idle_index][SEQ_NUM_OFF];
			ptr_handle->pri_mem.data_queue.recv_buf_num += 0x01; 
			
			
			if (ptr_handle->pri_mem.data_queue.recv_buf_num == QUEUE_RECV_SIZE)
			{
				LOG_UART_DEBUG("Recv buf is full, cannot fill any data. recv_buf_num = %d, queue_deep = %d\n", ptr_handle->pri_mem.data_queue.recv_buf_num, QUEUE_RECV_SIZE);
			}
		}	
	}


	if (!data_queue_is_empty(&(ptr_handle->pri_mem.data_queue), ENUM_RECV_QUEUE))
	{
		LOG_UART_DEBUG("Recv queue is not empty...\n");
		LOG_UART_DEBUG("Start timer...\n");

		ret_timer_code = ptr_handle->timer_start_cb(ptr_handle->p_timer_id, TIMER_TIMEOUT_MS, ptr_handle);
		if (ret_timer_code != RET_CODE_SUCESS)
		{
			LOG_UART_DEBUG("(%s)-->Start timer Failed...\n", __FUNCTION__);

			if (ptr_handle->pri_mem.parser_init_flag == MODULE_INIT)
			{
				ptr_handle->func_event_cb(ptr_handle, EVT_TIMER_FAILED, ptr_handle->pri_mem.send_seq_num, NULL, (uint16_t *)&(ptr_handle->pri_mem.send_len));
			}
		}
	}
	return 0;
}





int32_t uart_parser_init(void *handle, t_func_event_cb event_cb, t_func_send_cb send_cb)
{
	t_class_uart_parser *ptr_handle = (t_class_uart_parser *)handle;


	if ( (ptr_handle == NULL) || (event_cb == NULL) || (send_cb == NULL) )
	{
		return -ERR_PARAM_INVALID;
	}

	ptr_handle->func_event_cb = event_cb;
	ptr_handle->func_send_cb = send_cb;
	
	ptr_handle->func_cmd_send = uart_cmd_process;
	ptr_handle->func_recv_cb = uart_recv_process;

	ptr_handle->pri_mem.parser_init_flag = MODULE_INIT;


	return 0;
}




