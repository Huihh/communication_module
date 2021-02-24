
#include "uart_method.h"
#include "uart_constants.h"
#include "uart_log.h"



uint16_t calc_crc16(uint8_t *buf, uint16_t len)
{
	uint16_t crc16 = 0xFFFF;

    for (int i=0; i<len; i++)
    {
        crc16 ^= (uint16_t)buf[i];
        for (int j=8; j!= 0; j--) 
        {
            if ( (crc16 & 0x0001) != 0 )
            {
                crc16 >>= 1;
                crc16 ^= 0xA001;
            }
            else 
            {
                crc16 >>= 1;
            }
        }
    }


	crc16 = ((crc16 & 0x00ff) << 8) | ((crc16 & 0xff00) >> 8);
	
	return crc16;
}



uint8_t calc_checksum(uint8_t *buf, uint16_t len)
{
	uint8_t checksum;
	
	checksum = 0x00;
	for (int i=0; i<len; i++)
	{
		checksum += buf[i];
	}

	checksum = (~checksum + 1);
	
	return checksum;
}


uint32_t reassembly_packet(uint8_t seq_num, uint8_t func_code, uint8_t *dest, uint8_t *data, uint16_t data_len)
{
	uint16_t crc16;

	dest[START_CODE_OFF] = START_CODE;
	
	dest[DEV_TYPE_OFF] = 0x00;
	dest[DEV_TYPE_OFF + 1] = 0x01;
	
	dest[SEQ_NUM_OFF] = seq_num;
	
	dest[FUNC_CODE_OFF] = func_code;
	
	dest[DATA_LEN_OFF] 	 = (data_len >> 8) & 0xFF;
	dest[DATA_LEN_OFF + 1] = data_len & 0xFF;
	
	dest[CHECK_SUM_OFF] = calc_checksum(dest, CHECK_SUM_OFF);
	
	memmove(&dest[DATA_OFF], data, data_len);
	
	crc16 = calc_crc16(dest, (DATA_OFF + data_len));
	dest[DATA_OFF + data_len] = (crc16 >> 8) & 0xFF;
	dest[DATA_OFF + data_len + 1] = crc16 & 0xFF;
	
	return (data_len + PACKET_EXT_INFO);
}


bool data_queue_is_empty(t_data_queue *list, uint8_t which_queue)
{

	if (which_queue == ENUM_SEND_QUEUE)
	{
		if (list->send_buf_num == 0x00)
		{
			return true;
		}
	}
	else
	{
		if (list->recv_buf_num == 0x00)
		{
			return true;
		}
	}

	return false;
}


bool data_queue_is_full(t_data_queue *list, uint8_t which_queue)
{
	if (which_queue == ENUM_SEND_QUEUE)
	{
		if (list->send_buf_num == QUEUE_SEND_SIZE )
		{
			return true;
		}
	}
	else
	{
		if (list->recv_buf_num == QUEUE_RECV_SIZE)
		{
			return true;
		}
	}

	return false;
}


uint8_t get_idle_index(t_data_queue *list, uint8_t which_queue)
{
	uint8_t idle_index = 0xFF;

	if (which_queue == ENUM_SEND_QUEUE)
	{

		for (int i=0; i<QUEUE_SEND_SIZE; i++)
		{
			if (list->send_idle_index[i] == 0x00)
			{
				idle_index = i;
				break;
			}
		}
	}
	else
	{
		for (int i=0; i<QUEUE_RECV_SIZE; i++)
		{
			if (list->recv_idle_index[i] == 0x00)
			{
				idle_index = i;
				break;
			}
		}
	}

	return idle_index;
}


uint8_t search_seq_num_index(uint8_t buf[], uint8_t element_num, uint8_t seq_num)
{
	for (uint8_t i=0; i<element_num; i++)
	{
		if (buf[i] == seq_num)
		{
			return i;
		}
	}	

	return 0xFF;
}



uint8_t add_recv_seq_num(uint8_t buf[][RSP_ACK_MAX_LEN], uint8_t element_num, uint8_t seq_num)
{
	for (int i=0; i<element_num; i++)
	{
		if (buf[i][RSP_SEQ_NUM_OFF] == 0x00)
		{
			buf[i][RSP_SEQ_NUM_OFF] = seq_num;
			memset(buf[(i+1)%element_num], 0x00, RSP_ACK_MAX_LEN);
			return i;
		}
	}

	return 0xFF;
}


uint8_t remove_recv_seq_num(uint8_t buf[][RSP_ACK_MAX_LEN], uint8_t element_num, uint8_t seq_num)
{
	for (int i=0; i<element_num; i++)
	{
		if (buf[i][RSP_SEQ_NUM_OFF] == seq_num)
		{
			memset(buf[i], 0x00, RSP_ACK_MAX_LEN);
			return i;
		}
	}

	return 0xFF;
}



uint8_t search_recv_seq_num(uint8_t buf[][RSP_ACK_MAX_LEN], uint8_t element_num, uint8_t seq_num)
{
	for (uint8_t i=0; i<element_num; i++)
	{
		if (buf[i][RSP_SEQ_NUM_OFF] == seq_num)
		{
			return i;
		}
	}	

	return 0xFF;
}





bool verify_checksum(uint8_t *data, uint32_t data_len)
{

	uint8_t recv_checksum, local_checksum;

	if (data_len < CHECK_SUM_OFF)
	{
		return false;
	}
	
	recv_checksum = data[CHECK_SUM_OFF];
	local_checksum = calc_checksum(data, CHECK_SUM_OFF);

	if (recv_checksum != local_checksum)
	{
		return false;
	}


	return true;
}



bool verify_crc16(uint8_t *data, uint32_t data_len)
{

	uint16_t len, recv_crc16, local_crc16;
	if (data_len < CHECK_SUM_OFF)
	{
		return false;
	}

	len = data[DATA_LEN_OFF] << 8 | data[DATA_LEN_OFF + 1];

	if (data_len < (len + PACKET_EXT_INFO))
	{
		return false;
	}

	recv_crc16 = data[DATA_OFF + len] << 8 | data[DATA_OFF + len + 1];
	local_crc16 = calc_crc16(data, (DATA_OFF + len));

	if (recv_crc16 != local_crc16)
	{
		return false;
	}

	return true;
}





bool search_start_code(uint8_t *data, uint16_t *len)
{
	uint16_t i;
	for (i=0; i<*len; i++)
	{
		if (data[i] == START_CODE)
		{
			break;
		}
	}

	if (i == *len)
	{
		*len = 0;
		return false;
	}

	*len -= i;
	memmove(data, &data[i], *len);
	

	return true;
}
