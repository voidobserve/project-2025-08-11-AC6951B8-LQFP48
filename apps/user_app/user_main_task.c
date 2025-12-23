#include "user_main_task.h"

#include "user_sys_time.h"
#include "alarm.h" 
#include "user_schedule.h"

void user_main_task(void* p)
{
    // volatile user_sys_time_t time = { 2000, 01, 01 , 00 , 01 };
    // user_sys_time_set(&time);
    volatile user_sys_time_t time;
    user_sys_time_init();

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
            }
        }

        { // 每隔一段时间，打印一下当前系统时间
            static u16 cnt = 0;
            cnt++;
            if (cnt >= 500)
            {
                cnt = 0;
                user_sys_time_get(&time);
                printf("Current time: %d-%02d-%02d %02d:%02d:%02d\n",
                    time.year, time.month, time.day,
                    time.hour, time.min, time.sec);
                time.weekday = rtc_calculate_week_val(&time);
                printf("cur weekday: %d\n", time.weekday);
            }
        }
#endif

        instruction_scan();
        instruction_handle(); // 从串口缓冲区中取出数据，分析是否有指令，并进行处理

        save_user_data_time_count_down(); // 保存数据的倒计时，使能保存数据的操作才会进入内部执行
        save_user_data_handle(); // 保存用户数据，内部会根据标志位来判断是否保存
        os_time_dly(1); // 10ms

    }
}