
#include "test_function.h"


#include <signal.h> 
#include <unistd.h>
#include <strings.h>
#include <assert.h>

void printf_array(uint8_t *data, uint32_t data_len)
{
    for (int i=0; i<data_len; i++)
    {
        printf("%02x ", data[i]);
    }

    printf("\n");

}


int32_t event_handler(void *ptr_handle, uint8_t event_type, uint8_t seq_num, uint8_t *data, uint16_t *data_len)
{
    printf("EVENT HANDLER....\n");
    printf("event_type = %d, seq_num = 0x%02x, data_len = %d\n", event_type, seq_num, *data_len);
    printf_array(data, *data_len);

    /* Not yet complish by Huihh 2020.10.13 */
    return 0;
}




int32_t uart_send_data(void *ptr_handle, uint8_t *data, uint16_t data_len)
{
    printf("UART SEND DATA....\n");
    printf("data_len = %d\n", data_len);
    printf_array(data, data_len);

    /* Not yet complish by Huihh 2020.10.13 */
    return 0;
}


t_timer_handler_cb  expired_handler;
uint32_t timeout_s;
void *ptr_handler;

void sig_handler(int sig) 
{
    if (sig == SIGALRM) 
    {
        if (timeout_s == 0)
        {
            printf("timer stop....\n");
            return;
        }

        alarm (1); 

        printf("timer expired....\n");

        if (ptr_handler != NULL)
        {
            expired_handler(ptr_handler);
        }
        else
        {
            printf("(%s)-->ptr_handler null.....\n", __FUNCTION__);
        }
    }
}


uint8_t create_timer(void **p_timer_id, t_timer_handler_cb timeout_handler, timer_mode mode)
{

    expired_handler = timeout_handler;

    return 0;
}


uint8_t delete_timer(void *timer_id)
{

    expired_handler = NULL;

    return 0;
}


uint8_t start_timer(void *timer_id, uint32_t timeout_value, void *p_context)
{
    
    timeout_s = timeout_value;

    struct sigaction sa;
    bzero (&sa, sizeof (sa) );
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;

    ptr_handler = p_context;

    assert (sigaction (SIGALRM, &sa, NULL) != -1);
    alarm(1);



    return 0;
}


uint8_t stop_timer(void *timer_id)
{

    timeout_s = 0;

    return 0;
}

