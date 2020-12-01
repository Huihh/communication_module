
#ifndef __UART_LOG_H__
#define __UART_LOG_H__

#include <stdio.h>
#include <stdint.h>

#include "uart_transfer_api.h"

#ifdef __cplusplus
extern "C" {
#endif	



void LOG_UART_ARRAY_DEBUG(uint8_t *data, uint32_t data_len);










#ifdef __cplusplus
}
#endif	


#endif


