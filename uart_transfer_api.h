

#ifndef __UART_TRANSFER_API_H__
#define __UART_TRANSFER_API_H__
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "uart_constants.h"


#ifdef __cplusplus
extern "C" {
#endif	



/**
 * Log Macro define 
 * 
 * Legacy: Caller should accomplish log print terminal
 */
#define LOG_ENABLE

#ifdef LOG_ENABLE
	#define LOG_UART_DEBUG		printf
#else
	#define LOG_UART_DEBUG
#endif




/**
 * Enum define 
 * 
 */
typedef enum 
{
    TIMER_SINGLE_SHOT = 0x00,
    TIMER_REPEATED,

} timer_mode;


/**
 * protocol function code, when send command, only FUNC_CODE_READ, FUNC_CODE_WRITE, FUNC_CODE_REPORT used by upper layer
 * 
 */
typedef enum 
{
	FUNC_CODE_READ = 0x01,
	FUNC_CODE_WRITE,
	FUNC_CODE_ACK,
	FUNC_CODE_RSP,
	FUNC_CODE_REPORT,

}func_code_type_t;


/**
 * Event callback type, used to notification upper generate event
 * 
 */
typedef enum
{
	EVT_READ = FUNC_CODE_READ,
	EVT_WRITE = FUNC_CODE_WRITE,
	EVT_ACK = FUNC_CODE_ACK,
	EVT_RSP = FUNC_CODE_RSP,
	EVT_REPORT = FUNC_CODE_REPORT,
	EVT_TIMER_TIMEOUT,
	EVT_TIMER_FAILED,
	EVT_UART_SEND_ERR,

}evt_type_t;



/**
 * define function error code 
 * 
 */
typedef enum
{
    ERR_FUNC_CODE_INVALID = 0x01,		// Send or recv func_code invalid
    ERR_CALLBACK_UNINIT,				// Module uninit
    ERR_UART_SEND_ERR,                  // Uart send function return error
	ERR_PARAM_INVALID,					// Invalid api function parameter
	ERR_RECV_DATA_INVALID,				// Recv data invalid, beacuse checksum or crc16 failed
	ERR_NO_START_CODE,					// Not find start_code
	ERR_QUEUE_FULL,						// Send or recv data queue is full
	ERR_CAHCE_BUF_OVERFLOW,				// Cache buf overflow
	ERR_SEND_BUF_OVERFLOW,   			// Send buf overflow
	ERR_RECV_BUF_OVERFLOW, 				// Recv buf overflow


}err_code_t;


/**
 * Function pointer define 
 * 
 */


/**
 * define event callback function, used to notification upper
 * Direction: IN
 * User：Module
 */
typedef int32_t (*t_func_event_cb)(void *ptr_handle, uint8_t event_type, uint8_t seq_num, uint8_t *data, uint16_t *data_len);


/**
 * define uart send data callback function
 * Direction: IN
 * User：Module
 */
typedef int32_t (*t_func_send_cb)(void *ptr_handle, uint8_t *data, uint16_t data_len);


/**
 * define caller send command callback function
 * Direction: OUT
 * User：Caller (e.g. Upper or Application)
 */
typedef int32_t (*t_func_cmd_send)(void *ptr_handle, uint8_t func_code, uint8_t *data, uint16_t data_len);


/**
 * define process receive data callback function
 * Direction: OUT
 * User：Caller (e.g. Upper or Application)
 */
typedef int32_t (*t_func_recv_cb)(void *ptr_handle, uint8_t *data, uint16_t data_len);


/**
 * define timer expired callback function
 * Direction: IN
 * User：Module
 */
typedef void (*t_timer_handler_cb)(void *p_context);

/**
 * define create timer callback function
 * Direction: IN
 * User：Module
 */
typedef uint8_t (*t_timer_create_cb)(void **p_timer_id, t_timer_handler_cb timeout_handler, timer_mode mode);

/**uart_parser_init
 * define delete timer callback function
 * Direction: IN
 * User：Module
 */
typedef	uint8_t (*t_timer_delete_cb)(void *timer_id);

/**
 * define start timer callback function
 * Direction: IN
 * User：Module
 */
typedef	uint8_t (*t_timer_start_cb)(void *timer_id, uint32_t timeout_value, void *p_context);

/**
 * define stop timer callback function
 * Direction: IN
 * User：Module
 */
typedef	uint8_t (*t_timer_stop_cb)(void *timer_id);




/**
 * Structure define 
 * 
 */
typedef struct {


	t_pri_mem			pri_mem;			// Just used by module internal function, No external interface

	t_func_event_cb		func_event_cb;
	t_func_send_cb  	func_send_cb;
	
	t_func_cmd_send		func_cmd_send;
	t_func_recv_cb  	func_recv_cb;

	t_timer_create_cb 	timer_create_cb;
	t_timer_delete_cb 	timer_delete_cb;
	t_timer_start_cb  	timer_start_cb;
	t_timer_stop_cb   	timer_stop_cb;
	t_timer_handler_cb  timer_handler_cb;   // Just used by module internal function, No external interface
	void 			  	*p_timer_id;		// Just used by module internal function, No external interface

	
}t_class_uart_parser;








/**
 * API Function declare 
 * NOTE: The following API function must be initialized before using this module
 * NOTE: Module default parameters, If you wanna change one of them, you can get it in file <uart_constants.h>
 * 		  
 * UART_TX_BUF_MAX_LEN		0x40	//Buffer max size for send  data
 * UART_RX_BUF_MAX_LEN		0x40	//Buffer max size for recv data
 * CACHE_BUF_MAX_LEN		0x80	//Buffer max size for cache data
 * QUEUE_SEND_SIZE 		    0x05	//Send command queue deepth
 * QUEUE_RECV_SIZE 	        0x05    //Recv data queue deepth
 * TIMER_TIMEOUT_MS	        0x0A	//Timer resolution
 * TIMER_TIMEOUT_SLOT	    0x64    //Packet Timeout time = 10 * 100 = 1000ms = 1s
 * RETRY_MAX_TIMES		    0x03	//Send command retry times	 
 */

int32_t uart_parser_init(void *ptr_handle, t_func_event_cb event_cb, t_func_send_cb send_cb);
int32_t uart_timer_init(void *ptr_handle, t_timer_create_cb p_create, t_timer_delete_cb p_delete, t_timer_start_cb p_start, t_timer_stop_cb p_stop);






#ifdef __cplusplus
}
#endif	


#endif


