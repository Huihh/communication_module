
#include "uart_transfer_api.h"
#include "test_function.h"


#include "uart_method.h"

#include <strings.h>
#include <string.h>

uint8_t send_buf[32];
uint16_t send_len;


uint8_t recv_buf[64];
uint16_t recv_len;



int main()
{
    int32_t ret_code, send_count = 6;

    t_class_uart_parser uart_parser;
    bzero(&uart_parser, sizeof(uart_parser));

    ret_code = uart_parser_init((void *)&uart_parser, event_handler, uart_send_data);
    if (ret_code != 0)
    {
        LOG_UART_DEBUG("(%s)-->Init uart_parser_init failed.\n", __FUNCTION__);
    }

    LOG_UART_DEBUG("Init uart_parser_init successed\n");

    ret_code = uart_timer_init((void *)&uart_parser, create_timer, delete_timer, start_timer, stop_timer);
    if (ret_code != 0)
    {
        LOG_UART_DEBUG("(%s)-->uart_timer_init failed.\n", __FUNCTION__);
    }


    LOG_UART_DEBUG("Init uart_timer_init successed\n");


    for (int i=0; i<32; i++)
    {
        send_buf[i] = i;
    }

    send_len = 32;

    while (send_count >= 0)
    {
        ret_code = uart_parser.func_cmd_send((void *)&uart_parser, FUNC_CODE_READ, send_buf, send_len);
        if (ret_code < 0)
        {
            printf("(%s)-->send command failed, errno = %d\n", __FUNCTION__, -ret_code);
            break;
        }
        else
        {
            send_count -= 1;

            printf("send seq_num = %d\n", ret_code);
        }

    }


 

//    memmove(recv_buf, uart_parser.pri_mem.data_queue.send_buf[0], (recv_len = 42));
//
//    printf("Recv data len = %d\n", recv_len);
//    uart_parser.func_recv_cb((void *)&uart_parser, recv_buf, recv_len);




    LOG_UART_DEBUG("Hello world\n");

    while (1)
    {

    }

    return 0;
}

