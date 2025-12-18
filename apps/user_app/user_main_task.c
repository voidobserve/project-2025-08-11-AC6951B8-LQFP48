#include "user_main_task.h"

#include "user_sys_time.h"
#include "alarm.h" 
 
void user_main_task(void* p)
{
    user_sys_time_t time = { 2025, 12, 20 , 16 , 37 };
    user_sys_time_set(&time);

    while (1)
    {

        {
            static u8 cnt = 0;
            cnt++;
            if (cnt >= 100) // 100 --> 100 * 10ms，每1s进入一次
            {
                cnt = 0;

                user_sys_time_t time = { 0 };
                user_sys_time_get(&time);
            }
        } 

        instruction_scan();
        instruction_handle(); // 从串口缓冲区中取出数据，分析是否有指令，并进行处理
 
        save_user_data_time_count_down(); // 保存数据的倒计时，使能保存数据的操作才会进入内部执行
        save_user_data_handle(); // 保存用户数据，内部会根据标志位来判断是否保存
        os_time_dly(1); // 10ms

    }
}