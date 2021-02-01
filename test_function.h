
#ifndef __TEST_FUNCTION_H__
#define __TEST_FUNCTION_H__

#include <stdio.h>
#include <stdint.h>

#include "uart_transfer_api.h"


#ifdef __cplusplus
extern "C" {
#endif	


int32_t event_handler(void *ptr_handle, uint8_t event_type, uint8_t seq_num, uint8_t *data, uint16_t *data_len);
int32_t uart_send_data(void *ptr_handle, uint8_t *data, uint16_t data_len);



uint8_t create_timer(void **p_timer_id, t_timer_handler_cb timeout_handler, timer_mode mode);
uint8_t start_timer(void *timer_id, uint32_t timeout_value, void *p_context);
uint8_t delete_timer(void *timer_id);
uint8_t stop_timer(void *timer_id);



#ifdef __cplusplus
}
#endif	


#endif


