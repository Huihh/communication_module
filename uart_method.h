
#ifndef __UART_METHOD_H__
#define __UART_METHOD_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "uart_transfer_api.h"

#ifdef __cplusplus
extern "C" {
#endif	



uint16_t calc_crc16(uint8_t *buf, uint16_t len);
uint8_t calc_checksum(uint8_t *buf, uint16_t len);

uint32_t reassembly_packet(uint8_t seq_num, uint8_t func_code, uint8_t *dest, uint8_t *data, uint16_t data_len);

bool data_queue_is_empty(t_data_queue *list, uint8_t which_queue);
bool data_queue_is_full(t_data_queue *list, uint8_t which_queue);

bool verify_checksum(uint8_t *data, uint32_t data_len);
bool verify_crc16(uint8_t *data, uint32_t data_len);

bool search_start_code(uint8_t *data, uint16_t *len);



#ifdef __cplusplus
}
#endif	


#endif


