#include "user_main_task.h"

void user_main_task(void* p)
{

    while (1)
    {
        // u8 buff[] = { "user_main_task\n" };
        // Uart1_Send_Tx(buff, ARRAY_SIZE(buff));

        // uart_event_handler();


        os_time_dly(1); // 10ms

    }
}