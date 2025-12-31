#include "user_main_task.h"

#include "user_sys_time.h"
#include "alarm.h" 
#include "user_schedule.h"

#include "flash_driver.h"


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
        {
            // printf("recv msg MSG_USER_SAVE_INFO \n");
            save_user_data_enable(); // 使能保存数据的倒计时，使能保存数据的操作
        }
        break;

        }
    }
}

void user_main_task(void* p)
{
    // volatile user_sys_time_t time = { 2000, 01, 01 , 00 , 01 };
    // user_sys_time_set(&time);
    volatile user_sys_time_t time = { 0 };

    while (1)
    {
#if 1
        {
            static u8 cnt = 0;
            cnt++;
            // if (cnt >= 100) // 100 --> 100 * 10ms，每1s进入一次
            // if (cnt >= 50) // 50 --> 50 * 10ms，每 500 ms进入一次
            if (cnt >= 20) // 20 --> 20 * 10ms，每 200 ms进入一次
            {
                cnt = 0;
                weekly_schedule_info_handle(); // 可能需要每200ms扫描一次 
                weekly_schedule_relay_info_handle();
            }
        }

        { // 每隔一段时间，打印一下当前系统时间
            static u16 cnt = 0;
            cnt++;
            if (cnt >= 500)
            {
                cnt = 0;

#if 0
                user_sys_time_get(&time);
                printf("Current time: %d-%02d-%02d %02d:%02d:%02d\n",
                    time.year, time.month, time.day,
                    time.hour, time.min, time.sec);
                time.weekday = rtc_calculate_week_val(&time);
                printf("cur weekday: %d\n", time.weekday);
#endif

                // printf("\n=============================================\n");
                // printf("is sequencer in delay: %u\n", (u16)is_sequencer_in_delay()); 
            }
        }
#endif

        instruction_scan(); // 从串口缓冲区中取出数据，分析是否有指令 
        instruction_handle(); // 从指令缓冲区中取出指令，并进行处理 



        save_user_data_time_count_down(); // 保存数据的倒计时，使能保存数据的操作才会进入内部执行
        save_user_data_handle(); // 保存用户数据，内部会根据标志位来判断是否保存
        os_time_dly(1); // 10ms 
    }
}