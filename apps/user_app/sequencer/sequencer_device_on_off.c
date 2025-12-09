// 时序器的开机和关机
#include "sequencer_device_on_off.h"

#include "../../../apps/user_app/user_config.h"
#include "../../include_lib/system/timer.h" // 使用到系统提供的定时器接口
// #include "../../apps/user_app/flash_handle/flash_handle.h" // 读写flash接口



volatile int sequencer_power_on_timer_isr_id = 0;
volatile int sequencer_power_off_timer_isr_id = 0;

// u16 sequencer_power_on_timer_isr_id = 0;
// u16 sequencer_power_off_timer_isr_id = 0;

volatile int led_power_flash_timer_isr_id = 0; // 电源灯光闪烁的定时器id


/**
 * @brief 电源指示灯闪烁 任务
 *      调用前后，要确保电源指示灯的状态
 *      例如闪烁完成后，要让灯光一直点亮/熄灭
 *
 *      每隔一段时间调用一次，例如：每500ms调用一次
 *
 * @return * void
 */
void led_power_flash_task(void* p)
{
    power_light_toggle();
}

/**
 * @brief 确认时序器是否处于开关机的延时状态
 *
 * @return 0：时序器 没有处于 开关机的延时中
 *         1：时序器 处于 开关机的延时中
 *
 */
u8 is_sequencer_in_delay(void)
{
    return sequencers.is_in_delay;
}

/**
 * @brief 置位 时序器的开关机延时状态标志，
 *      表示时序器 处于 开关机的延时
 *
 */
void sequencer_flag_in_delay_set(void)
{
    sequencers.is_in_delay = 1;
}

/**
 * @brief 清除 时序器的开关机延时状态标志，
 *      表示时序器 没有处于 开关机的延时
 *
 */
void sequencer_flag_in_delay_clear(void)
{
    sequencers.is_in_delay = 0;
}



void sequencer_power_on_task(void* p)
{
    static u8 cur_relay_index = 0; // 当前继电器索引
    static u16 cur_relay_open_time = 0; // 当前继电器开机延时时间（单位：秒）
    static u8 flag_is_in_counting = 0; // 标志位，是否正在倒计时
 
#if 1 // 可能还要再优化压缩一下

    if (0 == flag_is_in_counting) // 如果不在倒计时中
    {
        // 每次进入，找到要开机的继电器，判断它的开机延时时间
        for (u8 i = 0; i < sequencers.relay_number; i++)
        {
            if (sequencers.relay[i].cur_status_on_off == RELAY_STATUS_DEACTIVE &&
                sequencers.relay[i].last_status_on_off == RELAY_STATUS_ACTIVE)
            {
                /*
                    如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                */
                cur_relay_index = i;

                // 得到 当前继电器的 开机延时时间

                if (sequencers.relay[i].open_time > 0)
                {
                    // 每次进入已经过了1s，这里要减一
                    cur_relay_open_time = sequencers.relay[i].open_time - 1;
                }
                else
                {
                    cur_relay_open_time = 0;
                }

                // cur_relay_open_time = sequencers.relay[i].open_time;

                flag_is_in_counting = 1;
                sequencer_flag_in_delay_set(); // 表示时序器正在开机的延时中（正在执行开机）
                break;
            }
        }

        // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该定时任务
        if (0 == flag_is_in_counting)
        {
            sys_hi_timer_del(sequencer_power_on_timer_isr_id);
            sys_hi_timer_del(led_power_flash_timer_isr_id);
            power_light_on(); // 点亮电源指示灯(开机完成后，电源指示灯常亮) 
            sequencer_flag_in_delay_clear();  // 时序器完成了开机延时，清空对应的标志位

            cur_relay_index = 0;
            sequencers.on_ff = DEVICE_ON; // 表示设备已经开机 

            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
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
            relay_status_setting(cur_relay_index, RELAY_STATUS_ACTIVE); // 开启继电器
            lcd_relay_icon_show(cur_relay_index); // lcd 显示对应的继电器图标

            // 更新 记录的 继电器的状态
            sequencers.relay[cur_relay_index].cur_status_on_off = RELAY_STATUS_ACTIVE;
            // sequencers.relay[cur_relay_index].last_status_on_off = RELAY_STATUS_ACTIVE;

            flag_is_in_counting = 0; 

            // 寻找下一个要开机的继电器 
            for (u8 i = cur_relay_index; i < sequencers.relay_number; i++)
            {
                if (sequencers.relay[i].cur_status_on_off == RELAY_STATUS_DEACTIVE &&
                    sequencers.relay[i].last_status_on_off == RELAY_STATUS_ACTIVE)
                {
                    /*
                        如果当前继电器是关着的，并且上次是开着的，则说明是要准备开机的继电器
                    */
                    cur_relay_index = i;
                    cur_relay_open_time = sequencers.relay[i].open_time;
                    flag_is_in_counting = 1;
                    sequencer_flag_in_delay_set(); // 表示时序器正在开机的延时中（正在执行开机）
                    break;
                }
            }

            // 如果运行到这里，并且没有在倒计时，说明所有继电器的开机延时都已经完成，关闭该定时任务
            if (0 == flag_is_in_counting)
            {
                sys_hi_timer_del(sequencer_power_on_timer_isr_id);
                sys_hi_timer_del(led_power_flash_timer_isr_id);
                power_light_on(); // 开启电源指示灯（开机后，电源指示灯常亮）
                sequencer_flag_in_delay_clear(); // 时序器完成了开机延时，清空对应的标志位

                cur_relay_index = 0;
                sequencers.on_ff = DEVICE_ON; // 表示设备已经开机 
                os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
            }
        }
    }
#endif


}

void sequencer_power_off_task(void* p)
{
#if 1
    static u8 cur_relay_index = 0; // 当前继电器索引
    static u8 cur_relay_close_time = 0; // 当前继电器关机延时时间
    static u8 flag_is_in_counting = 0; // 标志位，是否正在倒计时

    if (0 == flag_is_in_counting) // 如果不在倒计时中，找到要关机的继电器
    {
        for (int16_t i = sequencers.relay_number - 1; i >= 0; i--) // 关机顺序：从最后一个继电器，到第一个继电器
        {
            if (sequencers.relay[i].cur_status_on_off == RELAY_STATUS_ACTIVE)
            {
                // 如果当前继电器是开着的，说明是要准备关机的继电器
                cur_relay_index = i;

                if (sequencers.relay[i].close_time > 0)
                {
                    cur_relay_close_time = sequencers.relay[i].close_time - 1;
                }
                else
                {
                    cur_relay_close_time = 0;
                }

                flag_is_in_counting = 1;
                sequencer_flag_in_delay_set(); // 表示时序器正在关机的延时中（正在执行关机）
                break;
            }
        }

        // 如果运行到这里，并且没有在倒计时，说明所有继电器的关机延时都已经完成，关闭该定时任务
        if (0 == flag_is_in_counting)
        {
            sys_hi_timer_del(sequencer_power_off_timer_isr_id);
            sys_hi_timer_del(led_power_flash_timer_isr_id);
            power_light_off(); // 关闭电源指示灯（关机后，电源指示灯熄灭）
            sequencer_flag_in_delay_clear(); // 表示时序器执行完了关机延时
            sequencers.on_ff = DEVICE_OFF; // 表示设备已经关机
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
        }
    }

    if (flag_is_in_counting)
    {
        if (cur_relay_close_time > 0)
        {
            cur_relay_close_time--;
        }

        if (cur_relay_close_time == 0) // 如果减到0秒
        {
            //  
            // temp_on_off[cur_relay_index] = sequencers.relay[cur_relay_index].clod_on_off; // 继电器关机时对应的状态
            // relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关 

            relay_status_setting(cur_relay_index, RELAY_STATUS_DEACTIVE); // 继电器关闭
            lcd_relay_icon_unshow(cur_relay_index); // lcd 不显示对应的继电器图标
    
            sequencers.relay[cur_relay_index].cur_status_on_off = RELAY_STATUS_DEACTIVE; 
            flag_is_in_counting = 0;

            // 寻找下一个要关机的继电器 
            for (int16_t i = cur_relay_index; i >= 0; i--)
            {
                if (sequencers.relay[i].cur_status_on_off == RELAY_STATUS_ACTIVE)
                {
                    // 如果当前继电器是开着的，说明是要准备关机的继电器
                    cur_relay_index = i;
                    cur_relay_close_time = sequencers.relay[i].close_time;
                    flag_is_in_counting = 1;
                    sequencer_flag_in_delay_set(); // 表示时序器正在关机的延时中（正在执行关机）
                    break;
                }
            }

            // 如果运行到这里，并且没有在倒计时，说明所有继电器的关机延时都已经完成，关闭该定时任务
            if (0 == flag_is_in_counting)
            {
                sys_hi_timer_del(sequencer_power_off_timer_isr_id);
                sys_hi_timer_del(led_power_flash_timer_isr_id); 
                power_light_off(); // 关闭电源指示灯（关机后，电源指示灯熄灭）
                sequencer_flag_in_delay_clear(); // 表示时序器执行完了关机延时
                sequencers.on_ff = DEVICE_OFF; // 表示设备已经关机
                os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); 
            }
        }
    }
#endif


}




// 更新时序器最大的开机时间，在开机前调用，时间存放到 sequencers.open_timeing
void sequencer_max_power_on_time_update(void)
{
    u8 i = 0;
    u32 sequencer_power_on_time = 0;
    for (i = 0; i < sequencers.relay_number; i++)
    {
        if (sequencers.relay[i].last_status_on_off == RELAY_STATUS_ACTIVE)
        {
            // 如果时序器之前是开着的
            sequencer_power_on_time += sequencers.relay[i].open_time;
        }
    }

    sequencers.open_timeing = sequencer_power_on_time;
}

/**
 * @brief 更新时序器最大的关机时间，在关机前调用，时间存放到 sequencers.close_timeing
 *
 */
void sequencer_max_power_off_time_update(void)
{
    u8 i = 0;
    u32 sequencer_power_off_time = 0;
    for (i = 0; i < sequencers.relay_number; i++)
    {
        if (sequencers.relay[i].cur_status_on_off == RELAY_STATUS_ACTIVE)
        {
            // 如果时序器现在是开着的，说明是要关闭的继电器，累加他们的关机延时时间
            sequencer_power_off_time += sequencers.relay[i].close_time;
        }
    }

    sequencers.close_timeing = sequencer_power_off_time;
}


void sequencer_power_on(void)
{
    if (is_sequencer_in_delay())
    {
        // 如果正在开/关机的延时，直接返回
        return;
    }

    // sequencers.timeing_flag = 0; // 表示正在开/关机的延时
    sequencer_flag_in_delay_set();// 表示正在开/关机的延时

    // 开机，所有继电器默认都是关闭的，清空对应的状态：（否则会影响开机的相关判断）
    for (u8 i = 0; i < sequencers.relay_number; i++)
    {
        sequencers.relay[i].cur_status_on_off = RELAY_STATUS_DEACTIVE;
    }

    sequencer_max_power_on_time_update(); // 更新时序器总的开机时间（所有继电器的开机延时时间累加） 

    // 开机任务：
    sequencer_power_on_timer_isr_id = sys_hi_timer_add(NULL, sequencer_power_on_task, 1000); // 参数3，时间，单位：ms
    led_power_flash_timer_isr_id = sys_hi_timer_add(NULL, led_power_flash_task, 500);
}

void sequencer_power_off(void)
{
    if (is_sequencer_in_delay())
    {
        // 如果正在开/关机的延时，直接返回
        return;
    }

    // sequencers.timeing_flag = 0; // 表示正在开/关机的延时
    sequencer_flag_in_delay_set();// 表示正在开/关机的延时

    sequencer_max_power_off_time_update(); // 获取时序器最大的关机时间

    // 关机任务：
    sequencer_power_off_timer_isr_id = sys_hi_timer_add(NULL, sequencer_power_off_task, 1000);
    led_power_flash_timer_isr_id = sys_hi_timer_add(NULL, led_power_flash_task, 500); // 电源灯光闪烁动画
}