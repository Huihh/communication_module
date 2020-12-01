
#include "uart_log.h"


void LOG_UART_ARRAY_DEBUG(uint8_t *data, uint32_t data_len)
{
    for (uint32_t i=0; i<data_len; i++)
    {
        LOG_UART_DEBUG("%02x ", data[i]);
    }

    LOG_UART_DEBUG("\n");
}






