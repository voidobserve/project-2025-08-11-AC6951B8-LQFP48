#include "sequencer.h"
#include "../../user_app/flash_handle/flash_handle.h" // flash读写接口

volatile u8 flag_is_lcd_screen_on = 0; // 标志位，lcd屏幕状态，0--未点亮，1--点亮

volatile ON_OFF_FLAG temp_on_off[16];  //继电器的开关

volatile SEQUENCER  sequencers;



void user_msg_handle_task(void* p)
{
    int msg[32] = { 0 };


    while (1)
    {
        // os_sem_pend(msg, 0); // 一直阻塞等待信号量
        int ret = os_taskq_pend("msg_task", msg, 1);
        // printf("recv msg\n");
        // printf("ret %d\n", ret);
        if (OS_TASKQ != ret) 
        {
            continue;
        }

        if (msg[0] != Q_USER) // 如果不是用户消息
        {
            continue;
        }

        // for (u8 i =0; i  < ARRAY_SIZE(msg); i++)
        // {
        //     printf("msg [%u]: %d\n", (u16)i, msg[i]);
        // }


        switch (msg[1])
        {
        case MSG_USER_SAVE_INFO: // 如果要保存信息
            // printf("recv msg MSG_USER_SAVE_INFO \n");
            save_user_data_enable(); // 使能保存数据的倒计时，使能保存数据的操作
            break;

            // case MSG_SEQUENCER_POWER_ON:
            //     sequencer_power_on();
            //     break;

            // case MSG_SEQUENCER_POWER_OFF:
        }
    }



    // if (MSG_SEQUENCER_SAVE_INFO)

}