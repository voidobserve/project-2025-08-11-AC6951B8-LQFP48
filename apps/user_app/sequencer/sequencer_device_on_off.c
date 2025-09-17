// 时序器的开机和关机
#include "sequencer_device_on_off.h"

#include "../../include_lib/system/timer.h" // 使用到系统提供的定时器接口
// #include "../../apps/user_app/flash_handle/flash_handle.h" // 读写flash接口



volatile int sequencer_power_on_timer_isr_id = 0;
volatile int sequencer_power_off_timer_isr_id = 0;

// u16 sequencer_power_on_timer_isr_id = 0;
// u16 sequencer_power_off_timer_isr_id = 0;

volatile int led_power_flash_timer_isr_id = 0; // 电源灯光闪烁的定时器id




void led_power_flash_task(void)
{
    master_led_flashing();  // 总开关灯闪
}

// 第一次上电时，调用的开关机
void sequencer_first_power_on_task(void)
{
    static u8 cur_relay_index = 0; // 当前继电器索引
    static u8 cur_relay_open_time = 0; // 当前继电器开机延时时间
    static u8 flag_is_in_counting = 0; // 标志位，是否正在倒计时


    // printf("%s\n", __func__);
    if (0 == flag_is_in_counting) // 如果不在倒计时中
    {
        // 每次进入，找到要开机的继电器，判断它的开机延时时间
        for (u8 i = 0; i < sequencers.relay_number; i++)
        {
            if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF &&
                sequencers.realy[i].last_status_on_off == DEVICE_ON)
            {
                /*
                    如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                */
                cur_relay_index = i;
                // cur_relay_open_time = sequencers.realy[i].open_time;
                // 每次进入已经过了1s，这里要减一
                if (sequencers.realy[i].open_time > 0)
                {
                    cur_relay_open_time = sequencers.realy[i].open_time - 1;
                }
                else
                {
                    cur_relay_open_time = 0;
                }

                flag_is_in_counting = 1;
                sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
                break;
            }
        }

        // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该定时任务
        if (0 == flag_is_in_counting)
        {
            sys_hi_timer_del(sequencer_power_on_timer_isr_id);
            sys_hi_timer_del(led_power_flash_timer_isr_id);
            gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）
            sequencers.timeing_flag = 1; // 表示时序器执行完了开机延时

            cur_relay_index = 0;
            // cur_relay_open_time = 0;
            sequencers.on_ff = DEVICE_ON; // 表示设备已经开机
            // printf("%d\n", __LINE__);
            // printf("sequencers.on_ff %u\n", (u16)sequencers.on_ff);

            os_taskq_post("msg_task", 1, MSG_SEQUENCER_SAVE_INFO);
            // os_taskq_post("msg_task", 1, 197);
        }
    }

    if (flag_is_in_counting)
    {
        // 减去1秒
        if (cur_relay_open_time > 0)
        {
            cur_relay_open_time--;
        }

        if (cur_relay_open_time == 0) // 如果减到0秒
        {
            //  
            temp_on_off[cur_relay_index] = DEVICE_ON; // 继电器 开/关
            relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关
            // sequencers.realy[cur_relay_index].open_on_off = DEVICE_ON;
            sequencers.realy[cur_relay_index].cur_status_on_off = DEVICE_ON;
            sequencers.realy[cur_relay_index].last_status_on_off = DEVICE_ON;
            flag_is_in_counting = 0;


            // 寻找下一个要开机的继电器 
            for (u8 i = cur_relay_index; i < sequencers.relay_number; i++)
            {
                if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF &&
                    sequencers.realy[i].last_status_on_off == DEVICE_ON)
                {
                    /*
                        如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                    */
                    cur_relay_index = i;
                    cur_relay_open_time = sequencers.realy[i].open_time;
                    flag_is_in_counting = 1;
                    sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
                    break;
                }
            }

            // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该定时任务
            if (0 == flag_is_in_counting)
            {
                sys_hi_timer_del(sequencer_power_on_timer_isr_id);
                sys_hi_timer_del(led_power_flash_timer_isr_id);
                gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）
                sequencers.timeing_flag = 1; // 表示时序器执行完了开机延时

                cur_relay_index = 0;
                // cur_relay_open_time = 0;
                sequencers.on_ff = DEVICE_ON; // 表示设备已经开机
                // printf("%d\n", __LINE__);
                // printf("sequencers.on_ff %u\n", (u16)sequencers.on_ff);
                os_taskq_post("msg_task", 1, MSG_SEQUENCER_SAVE_INFO);
                // os_taskq_post("msg_task", 1, 197);
            }
        }
    }
}

// void sequencer_power_on_timer_isr(void)
// {
//     // 每次进入，发送消息给对应线程，表示过了1s
//     os_taskq_post(SEQUENCER_POWER_ON_TASK_NAME, 1, 1);
// }

void sequencer_power_on_task(void* p)
{
#if 1
    static u8 cur_relay_index = 0; // 当前继电器索引
    static u8 cur_relay_open_time = 0; // 当前继电器开机延时时间
    static u8 flag_is_in_counting = 0; // 标志位，是否正在倒计时


    // printf("%s\n", __func__);
    if (0 == flag_is_in_counting) // 如果不在倒计时中
    {
        // 每次进入，找到要开机的继电器，判断它的开机延时时间
        for (u8 i = 0; i < sequencers.relay_number; i++)
        {
            if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF &&
                sequencers.realy[i].last_status_on_off == DEVICE_ON)
                // if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF)
            {
                /*
                    如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                */
                cur_relay_index = i;
                // cur_relay_open_time = sequencers.realy[i].open_time;
                // 每次进入已经过了1s，这里要减一
                if (sequencers.realy[i].open_time > 0)
                {
                    cur_relay_open_time = sequencers.realy[i].open_time - 1;
                }
                else
                {
                    cur_relay_open_time = 0;
                }

                flag_is_in_counting = 1;
                sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
                break;
            }
        }

        // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该定时任务
        if (0 == flag_is_in_counting)
        {
            sys_hi_timer_del(sequencer_power_on_timer_isr_id);
            sys_hi_timer_del(led_power_flash_timer_isr_id);
            gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）
            sequencers.timeing_flag = 1; // 表示时序器执行完了开机延时

            cur_relay_index = 0;
            // cur_relay_open_time = 0;
            sequencers.on_ff = DEVICE_ON; // 表示设备已经开机
            // printf("%d\n", __LINE__);
            // printf("sequencers.on_ff %u\n", (u16)sequencers.on_ff);

            os_taskq_post("msg_task", 1, MSG_SEQUENCER_SAVE_INFO);
            // os_taskq_post("msg_task", 1, 197);
        }
    }
    // else // 如果正在倒计时

    if (flag_is_in_counting)
    {
        // if (cur_relay_open_time > 0) // 如果当前继电器的开机延时时间大于0
        // {
            // 减去1秒
        if (cur_relay_open_time > 0)
        {
            cur_relay_open_time--;
        }

        if (cur_relay_open_time == 0) // 如果减到0秒
        {
            //  
            temp_on_off[cur_relay_index] = DEVICE_ON; // 继电器 开/关
            relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关
            // sequencers.realy[cur_relay_index].open_on_off = DEVICE_ON;
            sequencers.realy[cur_relay_index].cur_status_on_off = DEVICE_ON;
            sequencers.realy[cur_relay_index].last_status_on_off = DEVICE_ON;
            flag_is_in_counting = 0;


            // 寻找下一个要开机的继电器 
            for (u8 i = cur_relay_index; i < sequencers.relay_number; i++)
            {
                if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF &&
                    sequencers.realy[i].last_status_on_off == DEVICE_ON)
                    // if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF)
                {
                    /*
                        如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                    */
                    cur_relay_index = i;
                    cur_relay_open_time = sequencers.realy[i].open_time;
                    flag_is_in_counting = 1;
                    sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
                    break;
                }
            }

            // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该定时任务
            if (0 == flag_is_in_counting)
            {
                sys_hi_timer_del(sequencer_power_on_timer_isr_id);
                sys_hi_timer_del(led_power_flash_timer_isr_id);
                gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）
                sequencers.timeing_flag = 1; // 表示时序器执行完了开机延时

                cur_relay_index = 0;
                // cur_relay_open_time = 0;
                sequencers.on_ff = DEVICE_ON; // 表示设备已经开机
                // printf("%d\n", __LINE__);
                // printf("sequencers.on_ff %u\n", (u16)sequencers.on_ff);
                os_taskq_post("msg_task", 1, MSG_SEQUENCER_SAVE_INFO);
                // os_taskq_post("msg_task", 1, 197);
            }
        }
        // }
    }
#endif

#if 0
    int msg[32];
    u8 cur_relay_index = 0; // 当前继电器索引
    u8 cur_relay_open_time = 0; // 当前继电器开机延时时间
    u8 flag_is_in_counting = 0; // 标志位，是否正在倒计时
    // 刚进入开机任务，找到要进行开机的继电器
    for (u8 i = 0; i < sequencers.relay_number; i++)
    {
        if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF &&
            sequencers.realy[i].last_status_on_off == DEVICE_ON)
        {
            /*
                如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
            */
            cur_relay_index = i;
            cur_relay_open_time = sequencers.realy[i].open_time;

            flag_is_in_counting = 1;
            sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
            break;
        }
    }

    sequencer_power_on_timer_isr_id = sys_hi_timer_add(NULL, sequencer_power_on_timer_isr, 1000);
    led_power_flash_timer_isr_id = sys_hi_timer_add(NULL, led_power_flash_task, 500); // 电源灯光闪烁动画

    while (1)
    {
        int ret = os_taskq_pend(SEQUENCER_POWER_ON_TASK_NAME, msg, 0); // 阻塞等待
        if (ret == 0 && msg[0] == 1)
        {
            if (cur_relay_open_time > 0)
            {
                cur_relay_open_time--;
            }

            if (cur_relay_open_time == 0) // 如果减到0秒
            {
                // 对应的继电器开机：
                temp_on_off[cur_relay_index] = sequencers.realy[cur_relay_index].open_on_off; // 继电器 开机时对应的状态
                relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关 
                sequencers.realy[cur_relay_index].cur_status_on_off = DEVICE_ON;
                sequencers.realy[cur_relay_index].last_status_on_off = DEVICE_ON;
                flag_is_in_counting = 0;

                // 寻找下一个要开机的继电器 
                for (u8 i = cur_relay_index; i < sequencers.relay_number; i++)
                {
                    if (sequencers.realy[i].cur_status_on_off == DEVICE_OFF &&
                        sequencers.realy[i].last_status_on_off == DEVICE_ON)
                    {
                        /*
                            如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                        */
                        cur_relay_index = i;
                        cur_relay_open_time = sequencers.realy[i].open_time;

                        flag_is_in_counting = 1;
                        sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
                        break;
                    }
                }

                // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该开机任务
                if (0 == flag_is_in_counting)
                {
                    // sys_hi_timer_del(sequencer_power_on_timer_isr_id);
                    sys_hi_timer_del(led_power_flash_timer_isr_id);
                    task_exit(SEQUENCER_POWER_ON_TASK_NAME);
                    gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）
                    sequencers.timeing_flag = 1; // 表示时序器执行完了开机延时

                    cur_relay_index = 0;
                    sequencers.on_ff == DEVICE_ON; // 表示设备已经开机
                }
            }
        }

    }
#endif
}

void sequencer_power_off_task(void)
{
#if 1
    static u8 cur_relay_index = 0; // 当前继电器索引
    static u8 cur_relay_close_time = 0; // 当前继电器关机延时时间
    static u8 flag_is_in_counting = 0; // 标志位，是否正在倒计时

    if (0 == flag_is_in_counting) // 如果不在倒计时中，找到要关机的继电器
    {
        for (int16_t i = sequencers.relay_number - 1; i >= 0; i--) // 关机顺序：从最后一个继电器，到第一个继电器
        {
            if (sequencers.realy[i].cur_status_on_off == DEVICE_ON)
            {
                // 如果当前继电器是开着的，说明是要准备关机的继电器
                cur_relay_index = i;
                // cur_relay_close_time = sequencers.realy[i].close_time;

                if (sequencers.realy[i].close_time > 0)
                {
                    cur_relay_close_time = sequencers.realy[i].close_time - 1;
                }
                else
                {
                    cur_relay_close_time = 0;
                }
                flag_is_in_counting = 1;
                sequencers.timeing_flag = 0;// 表示时序器正在关机的延时中（正在执行关机）
                break;
            }
        }

        // 如果运行到这里，并且没有在倒计时，说明所有继电器的关机延时都已经完成，关闭该定时任务
        if (0 == flag_is_in_counting)
        {
            sys_hi_timer_del(sequencer_power_off_timer_isr_id);
            sys_hi_timer_del(led_power_flash_timer_isr_id);
            gpio_direction_output(sw0_led, 0); // 关闭电源对应的按键灯
            sequencers.timeing_flag = 1; // 表示时序器执行完了关机延时
            sequencers.on_ff = DEVICE_OFF; // 表示设备已经关机
            os_taskq_post("msg_task", 1, MSG_SEQUENCER_SAVE_INFO);
            // os_taskq_post("msg_task", 1, 197);
        }
    }
    // else // 如果正在倒计时

    if (flag_is_in_counting)
    {
        if (cur_relay_close_time > 0)
        {
            cur_relay_close_time--;
        }

        if (cur_relay_close_time == 0) // 如果减到0秒
        {
            //  
            temp_on_off[cur_relay_index] = sequencers.realy[cur_relay_index].clod_on_off; // 继电器关机时对应的状态
            relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关 
            sequencers.realy[cur_relay_index].cur_status_on_off = DEVICE_OFF;
            // sequencers.realy[cur_relay_index].last_status_on_off = DEVICE_OFF;
            flag_is_in_counting = 0;

            // 寻找下一个要关机的继电器 
            for (int16_t i = cur_relay_index; i >= 0; i--)
            {
                if (sequencers.realy[i].cur_status_on_off == DEVICE_ON)
                {
                    // 如果当前继电器是开着的，说明是要准备关机的继电器
                    cur_relay_index = i;
                    cur_relay_close_time = sequencers.realy[i].close_time;
                    flag_is_in_counting = 1;
                    sequencers.timeing_flag = 0; // 表示时序器正在开机的延时中（正在执行开机）
                    break;
                }
            }

            // 如果运行到这里，并且没有在倒计时，说明所有继电器的关机延时都已经完成，关闭该定时任务
            if (0 == flag_is_in_counting)
            {
                sys_hi_timer_del(sequencer_power_off_timer_isr_id);
                sys_hi_timer_del(led_power_flash_timer_isr_id);
                gpio_direction_output(sw0_led, 0); // 关闭电源对应的按键灯
                sequencers.timeing_flag = 1; // 表示时序器执行完了关机延时
                sequencers.on_ff = DEVICE_OFF; // 表示设备已经关机
                os_taskq_post("msg_task", 1, MSG_SEQUENCER_SAVE_INFO);
                // os_taskq_post("msg_task", 1, 197);
            }
        }
    }
#endif


}




// 更新时序器最大的开机时间，在开机前调用，时间存放到 sequencers.open_timeing
void sequencer_update_max_power_on_time(void)
{
    u8 i = 0;
    u16 sequencer_power_on_time = 0;
    for (i = 0; i < sequencers.relay_number; i++)
    {
        if (sequencers.realy[i].last_status_on_off == DEVICE_ON)
        {
            // 如果时序器之前是开着的
            sequencer_power_on_time += sequencers.realy[i].open_time;
        }
    }

    sequencers.open_timeing = sequencer_power_on_time;
}

/**
 * @brief 更新时序器最大的关机时间，在关机前调用，时间存放到 sequencers.close_timeing
 *
 */
void sequencer_update_max_power_off_time(void)
{
    u8 i = 0;
    u16 sequencer_power_off_time = 0;
    for (i = 0; i < sequencers.relay_number; i++)
    {
        if (sequencers.realy[i].cur_status_on_off == DEVICE_ON)
        {
            // 如果时序器现在是开着的，说明是要关闭的继电器，累加他们的关机延时时间
            sequencer_power_off_time += sequencers.realy[i].open_time;
        }
    }

    sequencers.close_timeing = sequencer_power_off_time;
}

void sequencer_first_power_on(void)
{
    if (sequencers.timeing_flag == 0)
    {
        // 如果正在开/关机的延时，直接返回
        return;
    }

    sequencers.timeing_flag = 0; // 表示正在开/关机的延时

    // 开机，所有继电器默认都是关闭的，清空对应的状态：（否则会影响开机的相关判断）
    for (u8 i = 0; i < sequencers.relay_number; i++)
    {
        sequencers.realy[i].cur_status_on_off = DEVICE_OFF;
    }

    sequencer_update_max_power_on_time(); // 更新时序器总的开机时间（所有继电器的开机延时时间累加）

    // 开机任务：
    sequencer_power_on_timer_isr_id = sys_hi_timer_add(NULL, sequencer_first_power_on_task, 1000); // 参数3，时间，单位：ms
    led_power_flash_timer_isr_id = sys_hi_timer_add(NULL, led_power_flash_task, 500);
}

void sequencer_power_on(void)
{
    if (sequencers.timeing_flag == 0)
    {
        // 如果正在开/关机的延时，直接返回
        return;
    }

    sequencers.timeing_flag = 0; // 表示正在开/关机的延时

    // 开机，所有继电器默认都是关闭的，清空对应的状态：（否则会影响开机的相关判断）
    for (u8 i = 0; i < sequencers.relay_number; i++)
    {
        sequencers.realy[i].cur_status_on_off = DEVICE_OFF;
        // sequencers.realy[i].last_status_on_off = DEVICE_OFF;
    }

    sequencer_update_max_power_on_time(); // 更新时序器总的开机时间（所有继电器的开机延时时间累加） 

    // 开机任务：
    sequencer_power_on_timer_isr_id = sys_hi_timer_add(NULL, sequencer_power_on_task, 1000); // 参数3，时间，单位：ms
    led_power_flash_timer_isr_id = sys_hi_timer_add(NULL, led_power_flash_task, 500);
}

void sequencer_power_off(void)
{
    if (sequencers.timeing_flag == 0)
    {
        // 如果正在开/关机的延时，直接返回
        return;
    }

    sequencers.timeing_flag = 0; // 表示正在开/关机的延时

    sequencer_update_max_power_off_time(); // 获取时序器最大的关机时间

    // 关机任务：
    sequencer_power_off_timer_isr_id = sys_hi_timer_add(NULL, sequencer_power_off_task, 1000);
    led_power_flash_timer_isr_id = sys_hi_timer_add(NULL, led_power_flash_task, 500); // 电源灯光闪烁动画
}