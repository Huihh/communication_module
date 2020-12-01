

#include "uart_timer.h"
#include "uart_constants.h"
#include "uart_method.h"
#include "uart_transfer_api.h"

#include "uart_log.h"




static void timer_expired_handler(void *p_context)
{

	uint8_t func_code, seq_num, temp_func_code, ret_timer_code;
	uint16_t len, packet_len, i, temp_send_len;
	int32_t ret_code;
	uint8_t temp_send_buf[UART_TX_BUF_MAX_LEN];

	uint8_t *temp_ptr, *data_ptr;

	bool process_flag = false;

	t_class_uart_parser *ptr_handle = (t_class_uart_parser *)p_context;

	LOG_UART_DEBUG("Timer expired hander...\n");

	LOG_UART_DEBUG("Handle timer list...\n");

	for (i=0; i<QUEUE_SEND_SIZE; i++)
	{

		if (ptr_handle->pri_mem.data_queue.send_waitting_rsp[i] == 0x01)
		{
			process_flag = true;

			LOG_UART_DEBUG("index = %d waitting rsp, curr_retry = %d...\n", i, ptr_handle->pri_mem.data_queue.send_retry_times[i]);

			if (ptr_handle->pri_mem.data_queue.send_time_expired_stamp[i] == 0x00)
			{
				LOG_UART_DEBUG("index = %d time expired...\n", i);

				seq_num = ptr_handle->pri_mem.data_queue.send_buf[i][SEQ_NUM_OFF];
				data_ptr = &(ptr_handle->pri_mem.data_queue.send_buf[i][DATA_OFF]);
				temp_ptr = ptr_handle->pri_mem.data_queue.send_buf[i];

				len = temp_ptr[DATA_LEN_OFF] << 8 | temp_ptr[DATA_LEN_OFF + 1];

				if (ptr_handle->pri_mem.data_queue.send_retry_times[i] == 0x00)
				{
					ptr_handle->pri_mem.data_queue.send_take_index = (ptr_handle->pri_mem.data_queue.send_take_index + 1) % QUEUE_SEND_SIZE;
					ptr_handle->pri_mem.data_queue.send_buf_overflow = 0x00;

					ptr_handle->func_event_cb(ptr_handle, EVT_TIMER_TIMEOUT, seq_num, data_ptr, &len);
					ptr_handle->pri_mem.data_queue.send_waitting_rsp[i] = 0x00;

					process_flag = false;
				}
				else
				{
					ptr_handle->pri_mem.data_queue.send_retry_times[i] -= 0x01;
					ptr_handle->pri_mem.data_queue.send_time_expired_stamp[i] = TIMER_TIMEOUT_SLOT;

					packet_len = len + PACKET_EXT_INFO;
					ret_code = ptr_handle->func_send_cb(ptr_handle, ptr_handle->pri_mem.data_queue.send_buf[i], packet_len);
					if (ret_code != RET_CODE_SUCESS)
					{
						ptr_handle->func_event_cb(ptr_handle, EVT_UART_SEND_ERR, seq_num, ptr_handle->pri_mem.data_queue.send_buf[i], &packet_len);
						LOG_UART_DEBUG("(%s)-->Send uart data failed, ret_code = %d\n", __FUNCTION__, ret_code);
					}
				}	
			}
			else
			{
				LOG_UART_DEBUG("index = %d time stamp = %d...\n", i, ptr_handle->pri_mem.data_queue.send_time_expired_stamp[i]);
				ptr_handle->pri_mem.data_queue.send_time_expired_stamp[i] -= 0x01;
			}
		}
	}


	LOG_UART_DEBUG("Handle recv data queue list start...\n");
	while (!data_queue_is_empty(&(ptr_handle->pri_mem.data_queue), ENUM_RECV_QUEUE))
	{
		func_code = ptr_handle->pri_mem.data_queue.recv_buf[ptr_handle->pri_mem.data_queue.recv_take_index][FUNC_CODE_OFF];
		seq_num   = ptr_handle->pri_mem.data_queue.recv_buf[ptr_handle->pri_mem.data_queue.recv_take_index][SEQ_NUM_OFF];


		LOG_UART_DEBUG("recv_give_index = %d, recv_take_index = %d\n", ptr_handle->pri_mem.data_queue.recv_give_index, ptr_handle->pri_mem.data_queue.recv_take_index);

		temp_ptr   = ptr_handle->pri_mem.data_queue.recv_buf[ptr_handle->pri_mem.data_queue.recv_take_index];
		len        = temp_ptr[DATA_LEN_OFF] << 8 | temp_ptr[DATA_LEN_OFF + 1];
		packet_len = len + PACKET_EXT_INFO;
		data_ptr   = &(ptr_handle->pri_mem.data_queue.recv_buf[ptr_handle->pri_mem.data_queue.recv_take_index][DATA_OFF]);


		LOG_UART_ARRAY_DEBUG(ptr_handle->pri_mem.data_queue.recv_buf[ptr_handle->pri_mem.data_queue.recv_take_index], packet_len);
		LOG_UART_DEBUG("func_code = 0x%02x, seq_num = 0x%02x, len = 0x%04x\n", func_code, seq_num, len);

		if ( (func_code == FUNC_CODE_READ) || (func_code == FUNC_CODE_WRITE) || (func_code == FUNC_CODE_REPORT) )
		{

			LOG_UART_DEBUG("Request...\n");

			temp_func_code = (func_code == FUNC_CODE_READ) ? FUNC_CODE_RSP : FUNC_CODE_ACK;

			LOG_UART_DEBUG("Event call upper layer...\n");
			LOG_UART_ARRAY_DEBUG(data_ptr, len);

			ret_code       = ptr_handle->func_event_cb(ptr_handle, func_code, seq_num, data_ptr, &len);
			temp_func_code = (ret_code == RET_CODE_SUCESS) ? temp_func_code : FUNC_CODE_ACK;

			LOG_UART_DEBUG("Upper layer fill data...\n");
			LOG_UART_ARRAY_DEBUG(data_ptr, len);

			memset(temp_send_buf, 0x00, UART_TX_BUF_MAX_LEN);

			LOG_UART_DEBUG("Reassembly packet...\n");
			
			len = (temp_func_code == FUNC_CODE_ACK) ? 1 : (len % (UART_TX_BUF_MAX_LEN - PACKET_EXT_INFO)) ;
			temp_send_len = reassembly_packet(seq_num, temp_func_code, temp_send_buf, data_ptr, len);
			
			LOG_UART_DEBUG("Call uart send packet...\n");
			LOG_UART_ARRAY_DEBUG(temp_send_buf, temp_send_len);

			ret_code = ptr_handle->func_send_cb(ptr_handle, temp_send_buf, temp_send_len);
			if (ret_code != RET_CODE_SUCESS)
			{
				ptr_handle->func_event_cb(ptr_handle, EVT_UART_SEND_ERR, seq_num, temp_send_buf, &temp_send_len);
				LOG_UART_DEBUG("(%s)-->Send uart data failed, ret_code = %d\n", __FUNCTION__, ret_code);
			}
			else
			{
				LOG_UART_DEBUG("Send uart data successed\n");
			}

		}
		else if ( (func_code == FUNC_CODE_ACK) || (func_code == FUNC_CODE_RSP) )
		{		

			LOG_UART_DEBUG("Rsponse...\n");

			if (ptr_handle->pri_mem.data_queue.send_buf[ptr_handle->pri_mem.data_queue.send_take_index][SEQ_NUM_OFF] == seq_num)
			{
				ptr_handle->func_event_cb(ptr_handle, func_code, seq_num, data_ptr, &len);
				ptr_handle->pri_mem.data_queue.send_waitting_rsp[ptr_handle->pri_mem.data_queue.send_take_index] = 0x00;

				ptr_handle->pri_mem.data_queue.send_take_index = (ptr_handle->pri_mem.data_queue.send_take_index + 1) % QUEUE_SEND_SIZE;
				ptr_handle->pri_mem.data_queue.send_buf_overflow = 0x00;
			}
			else
			{
				LOG_UART_DEBUG("(%s)-->recv ack not expect seq_num, ignore it, expect_seq_num = %d, recv_seq_num = %d\n", __FUNCTION__, ptr_handle->pri_mem.data_queue.send_buf[ptr_handle->pri_mem.data_queue.send_take_index][SEQ_NUM_OFF], seq_num);
			}

		}
		else
		{
			LOG_UART_DEBUG("(%s)-->Func_code is illegal, ignore it\n", __FUNCTION__);
		}

		ptr_handle->pri_mem.data_queue.recv_take_index = (ptr_handle->pri_mem.data_queue.recv_take_index + 1) % QUEUE_RECV_SIZE;
		ptr_handle->pri_mem.data_queue.recv_buf_overflow = 0x00;

	}


	if (false == process_flag)
	{
		LOG_UART_DEBUG("There has no command waiting rsp, stop timer...\n");

		ret_timer_code = ptr_handle->timer_stop_cb(ptr_handle->p_timer_id);
		if (ret_timer_code != RET_CODE_SUCESS) 
		{
			LOG_UART_DEBUG("(%s)-->Create timer Failed...\n", __FUNCTION__);

			if (ptr_handle->pri_mem.parser_init_flag == MODULE_INIT)
			{
				ptr_handle->func_event_cb(ptr_handle, EVT_TIMER_FAILED, ptr_handle->pri_mem.send_seq_num, NULL, (uint16_t *)&(ptr_handle->pri_mem.send_len));
			}
		}
	}
	else
	{
		LOG_UART_DEBUG("There has command waiting rsp, continue timer...\n");
	}
	

	LOG_UART_DEBUG("Handle recv data queue list over...\n");
}





int32_t uart_timer_init(void *handle, t_timer_create_cb p_create, t_timer_delete_cb p_delete, t_timer_start_cb p_start, t_timer_stop_cb p_stop)
{
	uint8_t ret_timer_code;

	t_class_uart_parser *ptr_handle = (t_class_uart_parser *)handle;

	if ( (ptr_handle == NULL) || (p_create == NULL) || (p_delete == NULL) || (p_start == NULL) || (p_stop == NULL) )
	{
		return -ERR_PARAM_INVALID;
	}

	ptr_handle->timer_create_cb  = p_create;
	ptr_handle->timer_delete_cb  = p_delete;
	ptr_handle->timer_start_cb = p_start;
	ptr_handle->timer_stop_cb  = p_stop;
	ptr_handle->timer_handler_cb = timer_expired_handler;	


	ptr_handle->pri_mem.timer_init_flag = MODULE_INIT;

	LOG_UART_DEBUG("Create timer...\n");

	ret_timer_code = ptr_handle->timer_create_cb(&(ptr_handle->p_timer_id), ptr_handle->timer_handler_cb, TIMER_REPEATED);
	if (ret_timer_code != RET_CODE_SUCESS) 
	{

		LOG_UART_DEBUG("(%s)-->Create timer Failed...\n", __FUNCTION__);

		if (ptr_handle->pri_mem.parser_init_flag == MODULE_INIT)
		{
			ptr_handle->func_event_cb(ptr_handle, EVT_TIMER_FAILED, ptr_handle->pri_mem.send_seq_num, NULL, (uint16_t *)&(ptr_handle->pri_mem.send_len));
		}
	}

	return 0;
}







