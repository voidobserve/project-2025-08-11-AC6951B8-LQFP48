#include "instruction_handler_func.h"
#include "sequencer.h"
#include "user_sys_time.h"
#include "instruction_feedback.h"

/**
 * @brief 收到对应的串口指令后，控制时序器开关机
 *
 * @param sequencer_addr  时序器地址
 * @param cmd   0x01: 开启设备  0x00: 关闭设备
 *
*/
void handle_device_on_off(u8 sequencer_addr, u8 cmd)
{
    if (0)
    {
        // USER_TO_DO
        // 如果地址不一样，直接退出 
        return;
    }

    if (sequencers.is_in_delay)
    {
        // 如果设备正在开关机的延时中 
        return;
    }

    if (0x01 == cmd)
    {
        // 如果是 开启设备 的命令
        if (DEVICE_ON == sequencers.on_ff)
        {
            // 如果设备已经开启 
            return;
        }

        // 进入到这里，说明设备没有开启，并且设备不处于开关机的延时中
        sequencer_power_on();
    }
    else if (0x00 == cmd)
    {
        // 如果是 关闭设备 的命令
        if (DEVICE_OFF == sequencers.on_ff)
        {
            // 如果设备已经关闭 
            return;
        }

        // 进入到这里，说明设备没有关闭，并且设备不处于开关机的延时中
        sequencer_power_off();
    }
    else
    {
        // USER_TO_DO
        // 如果命令格式有误
    }
}

/**
 * @brief  收到对应的串口指令后，处理时序器继电器状态设置
 *
 * @param sequencer_addr 时序器地址
 * @param relay_index 时序器中的继电器索引
 * @param relay_status 要设置的继电器状态
 */
void handle_relay_status_setting(u8 sequencer_addr, u8 relay_index, u8 relay_status)
{
    // 如果 设备ID 是指定所有设备
    // if (0xFF == sequencer_addr)
    if (0)
    {
        // USER_TO_DO 
        // 如果设备地址不一样，直接退出
        return;
    }

    if (DEVICE_OFF == sequencers.on_ff)
    {
        // 如果设备没有开启，则不能操作继电器 
        return;
    }

    if (relay_index > 8 || 0 == relay_index)
    {
        // 继电器的索引值越界
        return;
    }

    if (0x01 == relay_status)
    {
        sequencer_relay_status_setting_dly(relay_index - 1, RELAY_STATUS_ACTIVE, 50 / 10);
    }
    else if (0x00 == relay_status)
    {
        sequencer_relay_status_setting_dly(relay_index - 1, RELAY_STATUS_DEACTIVE, 50 / 10);
    }
    else
    {
        // USER_TO_DO
        // 格式有误
    }

}

/**
 * @brief 收到对应的串口指令后，设置继电器对应的 激活时间
 *
 * @param sequencer_addr
 * @param relay_index
 * @param active_time
 */
void handle_relay_active_time(u8 sequencer_addr, u8 relay_index, u16 active_time)
{
    if (0)
    {
        // USER_TO_DO
        // 如果地址不一样，直接退出
        return;
    }

    if (sequencers.is_in_delay)
    {
        // 如果设备正在开关机的延时中，不进行设置
        return;
    }

    if (relay_index < 1 || relay_index > 8)
    {
        // 如果继电器索引值超出了范围，不进行设置
        return;
    }

    if (active_time < 1 || active_time > 999)
    {
        // 如果激活时间不在 1 ~ 999 秒，不进行设置
        return;
    }

    sequencers.relay[relay_index - 1].open_time = active_time;
}

/**
 * @brief 收到对应的串口指令后，设置继电器对应的 停用时间
 *
 * @param sequencer_addr
 * @param relay_index
 * @param deactive_time
 */
void handle_relay_deactive_time(u8 sequencer_addr, u8 relay_index, u16 deactive_time)
{
    if (0)
    {
        // USER_TO_DO
        // 如果地址不一样，直接退出
        return;
    }

    if (sequencers.is_in_delay)
    {
        // 如果设备正在开关机的延时中，不进行设置
        return;
    }

    if (relay_index < 1 || relay_index > 8)
    {
        // 如果继电器索引值超出了范围，不进行设置
        return;
    }

    if (deactive_time < 1 || deactive_time > 999)
    {
        // 如果停用时间不在 1 ~ 999 秒，不进行设置
        return;
    }

    sequencers.relay[relay_index - 1].close_time = deactive_time;
}

/**
 * @brief 收到对应的串口指令后，设置系统时间
 *
 * @param sequencer_addr 设备地址
 * @param time 要设置的时间，年、月、日、时、分、秒（不用设置星期，星期可以计算得出）
 */
void handle_set_sys_time(u8 sequencer_addr, user_sys_time_t time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return;
    }

    // 判断时间是否合法
    if (time.year < 2000 || time.year > 2099 || // 超出了 2000 ~ 2099 的范围
        time.month < 1 || time.month > 12 || // 超出了 1 ~ 12 的范围
        time.day < 1 || time.day > 31 || // 超出了 1 ~ 31 的范围
        time.hour > 23 || // 超出了 0 ~ 23 的范围
        time.min > 59 || // 超出了 0 ~ 59 的范围
        time.sec > 59 // 超出了 0 ~ 59 的范围
        )
    {
        // u8 buffer[20] = {0};
        // int len = sprintf(buffer, "time format error\n");
        // instruction_feedback_buffer(buffer, len);        
        return;
    }

    user_sys_time_set(&time);

    printf("set time ok \n");

    // 测试给上位机反馈信息：
    // u8 buffer[20] = {0};
    // int len = sprintf(buffer, "device id[%u]\n", (u16)sequencers.addr);
    // extern void instruction_feedback_buffer(u8 * buffer, u8 len);
    // printf("len %d \n", len);
    // instruction_feedback_buffer(buffer, len);

    // time.hour = hour;
    // time.min = minute;
}

/**
 * @brief 收到对应的串口指令后，设置时序器设备开机、关机时间
 * 
 * @param sequencer_addr 
 * @param power_on_time 定时开机时间
 * @param power_off_time 定时关机时间
 */
// void handle_set_time_to_switch_on_off(u8 sequencer_addr, user_sys_time_t power_on_time, user_sys_time_t power_off_time)
// 设置每周的开关机计划时间
void handle_set_weekly_schedule(u8 sequencer_addr, u8 weekday, user_sys_time_t power_on_time, user_sys_time_t power_off_time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return;
    }
    
    
}
