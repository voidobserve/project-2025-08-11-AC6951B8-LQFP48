#include "user_main_task.h"

extern void lcd_relay_icon_show(relay_index_t relay_index);
extern void lcd_relay_icon_unshow(relay_index_t relay_index); // lcd 不显示 对应的继电器图标

void user_main_task(void* p)
{

    while (1)
    {
        // {
        //     static u8 cnt = 0;
        //     cnt++;
        //     if (cnt >= 50)
        //     {
        //         cnt = 0;
        //         power_light_toggle();
        //     }
        // }
        // dev_ioctl(dev_handle, IOCTL_GET_SYS_TIME, (u32)time)


        instruction_scan();
        instruction_handle(); // 从串口缓冲区中取出数据，分析是否有指令，并进行处理


        save_user_data_time_count_down(); // 保存数据的倒计时，使能保存数据的操作才会进入内部执行
        save_user_data_handle(); // 保存用户数据，内部会根据标志位来判断是否保存
        os_time_dly(1); // 10ms

    }
}