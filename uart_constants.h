

#ifndef __UART_CONSTANTS_H__
#define __UART_CONSTANTS_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif	


#define START_CODE		0x55





#define START_OFF			0x00

#define START_CODE_OFF		(START_OFF)
#define START_CODE_SIZE		0x01

#define DEV_TYPE_OFF		(START_CODE_OFF + START_CODE_SIZE)
#define DEV_TYPE_SIZE		0x02

#define SEQ_NUM_OFF			(DEV_TYPE_OFF + DEV_TYPE_SIZE)
#define SEQ_NUM_SIZE		0x01

#define FUNC_CODE_OFF		(SEQ_NUM_OFF + SEQ_NUM_SIZE)
#define FUNC_CODE_SIZE		0x01

#define DATA_LEN_OFF		(FUNC_CODE_OFF + FUNC_CODE_SIZE)
#define DATA_LEN_SIZE		0x02

#define CHECK_SUM_OFF		(DATA_LEN_OFF + DATA_LEN_SIZE)
#define CHECK_SUM_SIZE		0x01

#define DATA_OFF			(CHECK_SUM_OFF + CHECK_SUM_SIZE)

#define CRC16_SIZE			0x02


#define PACKET_EXT_INFO		0x0A	




/**
 * Notification module status
 * 
 */
typedef enum
{
	MODULE_UNINT = 0x00,
	MODULE_INIT,

}module_init_flag_t;


/**
 * Internal function return code
 * 
 */
typedef enum
{
	RET_CODE_SUCESS = 0x00,
	RET_CODE_FAILED,

}ret_code_t;



/**
 * which queue enum value
 * 
 */
typedef enum
{
	ENUM_SEND_QUEUE = 0x00,
	ENUM_RECV_QUEUE,

}enum_queue_t;


/**
 * response or ack data offset
 * 
 */
#define RSP_SEQ_NUM_OFF		(START_OFF)
#define RSP_SEQ_NUM_SIZE	0x01

#define RSP_RSP_ACK_OFF		(RSP_SEQ_NUM_OFF + RSP_SEQ_NUM_SIZE)
#define RSP_RSP_ACK_SIZE	0x01

#define RSP_DATA_LEN_OFF	(RSP_RSP_ACK_OFF + RSP_RSP_ACK_SIZE)
#define RSP_DATA_LEN_SIZE	0x01

#define RSP_DATA_OFF	    (RSP_DATA_LEN_OFF + RSP_DATA_LEN_SIZE)



/**
 * Configure macro define 
 * 
 */
#define UART_TX_BUF_MAX_LEN		0x40	//Buffer max size for send  data
#define UART_RX_BUF_MAX_LEN		0x40	//Buffer max size for recv data
#define CACHE_BUF_MAX_LEN		0x80	//Buffer max size for cache data
#define RSP_ACK_MAX_LEN			0x40    //Buffer max size for response or ack data


#define QUEUE_SEND_SIZE 		0x05	//Send command queue deepth
#define QUEUE_RECV_SIZE 	    0x05    //Recv data queue deepth
#define RECV_CMD_SEQ_NUM_SIZE	0x05    //cache recv cmd seq num max number
#define RECV_RSP_SEQ_NUM_SIZE	0x05    //cache recv rsp seq num max number


#define TIMER_TIMEOUT_MS	    0x0A	//Timer resolution
#define TIMER_TIMEOUT_SLOT	    0x64    //Packet Timeout time = 10 * 100 = 1000ms = 1s
#define RETRY_MAX_TIMES		    0x03	//Send command retry times	 


/**
 * Internal structure define 
 * 
 */
typedef struct {

	uint8_t send_buf[QUEUE_SEND_SIZE][UART_TX_BUF_MAX_LEN];
	uint8_t send_waitting_rsp[QUEUE_SEND_SIZE];
	uint8_t send_time_expired_stamp[QUEUE_SEND_SIZE];
	uint8_t send_retry_times[QUEUE_SEND_SIZE];
	uint8_t send_idle_index[QUEUE_SEND_SIZE];
	uint8_t send_buf_num;

	uint8_t recv_buf[QUEUE_RECV_SIZE][UART_RX_BUF_MAX_LEN];
	uint8_t recv_idle_index[QUEUE_RECV_SIZE];
	uint8_t recv_cmd_seq_num[RECV_CMD_SEQ_NUM_SIZE][RSP_ACK_MAX_LEN];
	uint8_t recv_rsp_seq_num[RECV_RSP_SEQ_NUM_SIZE][RSP_ACK_MAX_LEN];

	uint8_t recv_buf_num;

}t_data_queue;


typedef struct {

	uint8_t				parser_init_flag;
	uint8_t				timer_init_flag;

	uint8_t 			cache_buf[CACHE_BUF_MAX_LEN];
	uint8_t 			send_seq_num;
	uint8_t 			recv_seq_num;	//no used yet 2020.10.12

	uint32_t 			recv_len;
	uint32_t 			send_len;

	t_data_queue		data_queue;

}t_pri_mem;















#ifdef __cplusplus
}
#endif	


#endif


