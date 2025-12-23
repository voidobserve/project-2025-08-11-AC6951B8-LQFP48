#include "instruction_handler_func.h"
#include "user_config.h"

#include "sequencer.h"
#include "user_sys_time.h"
#include "instruction_feedback.h"
#include "user_schedule.h"



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
int handle_set_sys_time(u8 sequencer_addr, user_sys_time_t time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    // 判断时间是否合法
    // if (time.year < 2000 || time.year > 2099 || // 超出了 2000 ~ 2099 的范围
    //     time.month < 1 || time.month > 12 || // 超出了 1 ~ 12 的范围
    //     time.day < 1 || time.day > 31 || // 超出了 1 ~ 31 的范围
    //     time.hour > 23 || // 超出了 0 ~ 23 的范围
    //     time.min > 59 || // 超出了 0 ~ 59 的范围
    //     time.sec > 59 // 超出了 0 ~ 59 的范围
    //     )
    if (!is_user_time_valid(time))
    {
        // USER_PRINTF_FUNC();
        printf("time invalid \n");
        return 2;
    }

    user_sys_time_set(&time);

#if USER_DEBUG_ENABLE
    printf("set time ok \n");
#endif

    // 测试给上位机反馈信息：
    // u8 buffer[20] = {0};
    // int len = sprintf(buffer, "device id[%u]\n", (u16)sequencers.addr);
    // extern void instruction_feedback_buffer(u8 * buffer, u8 len);
    // printf("len %d \n", len);
    // instruction_feedback_buffer(buffer, len);

    // time.hour = hour;
    // time.min = minute;

    return 0;
}

/**
 * @brief 收到对应的串口指令后，设置时序器设备每周的开机、关机时间
 *
 * @param sequencer_addr
 * @param power_on_time 定时开机时间
 * @param power_off_time 定时关机时间
 */
void handle_set_weekly_schedule(u8 sequencer_addr, u8 weekday, user_sys_time_t power_on_time, user_sys_time_t power_off_time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return;
    }

    if (weekday >= 7)
    {
        // 如果星期值超界，直接退出
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("weekday invalid \n");
#endif 
        return;
    }

    if (power_on_time.hour > 23 ||
        power_on_time.min > 59 ||
        power_on_time.sec > 59 ||
        power_off_time.hour > 23 ||
        power_off_time.min > 59 ||
        power_off_time.sec > 59)
    {
        // 如果时间不合法，直接退出
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("time invalid \n");
#endif
        return;
    }

    power_on_time.weekday = weekday;
    power_off_time.weekday = weekday;

    weekly_schedule_info_set(power_on_time, power_off_time);
#if USER_DEBUG_ENABLE
    printf("set schedule ok \n");
#endif
}

/**
 * @brief 收到对应的串口指令后，取消时序器每周对应星期值的定时开关机
 *
 * @param sequencer_addr 设备地址
 * @param weekday 星期值
 *
 * @return
 */
int handle_cancel_weekly_schedule(u8 sequencer_addr, u8 weekday)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (weekday >= 7)
    {
        // 如果星期值超界，直接退出
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("weekday invalid \n");
#endif
        return 2;
    }

    // printf("weekday %u\n", (u16)weekday);

    weekly_schedule_info_cancel(weekday);
#if USER_DEBUG_ENABLE
    printf("cancel schedule ok \n");
#endif

    return 0;
}

/**
 * @brief 收到对应的串口指令后，初始化时序器设备地址
 *
 * @param sequencer_addr 设备地址
 */
void handle_init_all_device_addr(u8 sequencer_addr)
{
    sequencers.addr = sequencer_addr;
}

/**
 * @brief 收到对应的串口指令后，复位时序器设备为出厂设置
 *
 * @param sequencer_addr 设备地址
 * @return int
 */
int handle_reset_to_factory_setting(u8 sequencer_addr)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("sequencer is in delay \n");
#endif
        return 2;
    }

    if (sequencers.on_ff == DEVICE_ON)
    {
        // 如果设备处于开机状态
        /*
            这里不能直接调用 sequencers_data_init() 函数，
            会把记录当前开启的继电器状态都变成默认值，
            导致后续无法关机
        */
        // sequencers_data_init();

        sequencers.addr = 1; // 默认设备地址为 1
        sequencers.on_ff = DEVICE_OFF; // 默认不要开机
        sequencers.relay_number = RELAYS_MAX; // 继电器数量
        sequencer_flag_in_delay_clear(); // 默认设备不处于开关机的延时中

        // 初始化继电器的开机延时和关机延时：
        for (u8 i = 0; i < RELAYS_MAX; i++)
        {
            sequencers.relay[i].open_time = 1;
        }

        for (u8 i = 0; i < RELAYS_MAX; i++)
        {
            sequencers.relay[i].close_time = 1;
        }

        sequencer_power_off();
    }
    else
    {
        // 如果当前设备已经关机，直接初始化相关变量
        sequencers_data_init();
    }


    return 0;
}
