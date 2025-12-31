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
int handle_device_on_off(u8 sequencer_addr, u8 cmd)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出 
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中 
        return 2;
    }

    if (0x01 == cmd)
    {
        // 如果是 开启设备 的命令
        if (DEVICE_ON == sequencers.on_ff)
        {
            // 如果设备已经开启 
            return 0;
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
            return 0;
        }

        // 进入到这里，说明设备没有关闭，并且设备不处于开关机的延时中
        sequencer_power_off();
    }
    else
    {
        // 如果命令格式有误
        return 3;
    }

    return 0;
}

/**
 * @brief  收到对应的串口指令后，处理时序器继电器状态设置
 *
 * @param sequencer_addr 时序器地址
 * @param relay_index 时序器中的继电器索引
 * @param relay_status 要设置的继电器状态
 */
int handle_relay_status_setting(u8 sequencer_addr, u8 relay_index, u8 relay_status)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果设备地址不一样，直接退出
        return 1;
    }

    if (DEVICE_OFF == sequencers.on_ff)
    {
        // 如果设备没有开启，则不能操作继电器 
        return 2;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 3;
    }

    if (relay_index > 8 || 0 == relay_index)
    {
        // 继电器的索引值越界
        return 4;
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
        // 格式有误
        return 5;
    }

    return 0;
}

/**
 * @brief 收到对应的串口指令后，设置继电器对应的 激活时间
 *
 * @param sequencer_addr
 * @param relay_index
 * @param active_time 单位：秒
 */
int handle_relay_active_time(u8 sequencer_addr, u8 relay_index, u16 active_time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    if (relay_index < 1 || relay_index > 8)
    {
        // 如果继电器索引值超出了范围，不进行设置
        return 3;
    }

    if (active_time < 1 || active_time > 999)
    {
        // 如果激活时间不在 1 ~ 999 秒，不进行设置
        return 4;
    }

    sequencers.relay[relay_index - 1].open_time = active_time;
    return 0;
}

/**
 * @brief 收到对应的串口指令后，设置继电器对应的 停用时间
 *
 * @param sequencer_addr
 * @param relay_index
 * @param deactive_time 单位：秒
 */
int handle_relay_deactive_time(u8 sequencer_addr, u8 relay_index, u16 deactive_time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    if (relay_index < 1 || relay_index > 8)
    {
        // 如果继电器索引值超出了范围，不进行设置
        return 3;
    }

    if (deactive_time < 1 || deactive_time > 999)
    {
        // 如果停用时间不在 1 ~ 999 秒，不进行设置
        return 4;
    }

    sequencers.relay[relay_index - 1].close_time = deactive_time;
    return 0;
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

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    // 判断时间是否合法
    if (!user_time_is_valid(time))
    {
#if USER_DEBUG_ENABLE
        printf("time invalid \n");
#endif
        return 3;
    }

    user_sys_time_set(&time);

#if USER_DEBUG_ENABLE
    printf("set time ok \n");
#endif 

    return 0;
}

/**
 * @brief 收到对应的串口指令后，设置时序器设备每周的开机、关机时间
 *
 * @param sequencer_addr
 * @param power_on_time 定时开机时间
 * @param power_off_time 定时关机时间
 */
int handle_set_weekly_schedule(u8 sequencer_addr, u8 weekday, user_sys_time_t power_on_time, user_sys_time_t power_off_time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    if (weekday >= 7)
    {
        // 如果星期值超界，直接退出
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("weekday invalid \n");
#endif 
        return 3;
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
        return 4;
    }

    power_on_time.weekday = weekday;
    power_off_time.weekday = weekday;

    weekly_schedule_info_set(power_on_time, power_off_time);
#if USER_DEBUG_ENABLE
    printf("set schedule ok \n");
#endif

    return 0;
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

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    if (weekday >= 7)
    {
        // 如果星期值超界，直接退出
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("weekday invalid \n");
#endif
        return 3;
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
int handle_init_all_device_addr(u8 sequencer_addr)
{
    if (sequencer_addr == 0xFF)
    {
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    sequencers.addr = sequencer_addr;

    return 0;
}

int handle_set_relay_weekly_schedule(
    u8 sequencer_addr,
    relay_index_t relay_index,
    u8 weekday,
    user_sys_time_t active_time,
    user_sys_time_t deactive_time)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    if (weekday >= 7)
    {
        // 如果星期值超界，直接退出
#if USER_DEBUG_ENABLE
        USER_PRINTF_FUNC();
        printf("weekday invalid \n");
#endif 
        return 3;
    }

    if (relay_index == 0 || relay_index > 8)
    {
        // 如果继电器索引值超出了范围，直接退出
        return 4;
    }

    if (active_time.hour > 23 ||
        active_time.min > 59 ||
        active_time.sec > 59 ||

        deactive_time.hour > 23 ||
        deactive_time.min > 59 ||
        deactive_time.sec > 59)
    {
        // 如果时间不合法，直接退出
        return 5;
    }

    // 指令传入的 relay_index 范围是 1~8，而程序内部的 relay_index 范围是 0~7，这里要减去1
    week_schedule_relay_set_by_weekday(relay_index - 1, weekday, active_time, deactive_time);

    return 0;
}

int handle_cancel_relay_weekly_schedule(u8 sequencer_addr, u8 relay_index, u8 weekday)
{
    if (sequencer_addr != 0xFF && sequencer_addr != sequencers.addr)
    {
        // 如果地址不一样，直接退出
        return 1;
    }

    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，不进行设置
        return 2;
    }

    if (weekday >= 7)
    {
        // 如果星期值超界，直接退出 
        return 3;
    }

    if (relay_index == 0 || relay_index > 8)
    {
        // 如果继电器索引值超出了范围，直接退出
        return 4;
    }

    // 指令传入的 relay_index 范围是 1~8，而程序内部的 relay_index 范围是 0~7，这里要减去1
    weekly_schedule_relay_cancel_by_weekday(relay_index - 1, weekday);

#if USER_DEBUG_ENABLE
    printf("cancel relay schedule ok \n");
#endif

    return 0;
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

        sequencer_power_off(); // 执行关机操作
    }
    else
    {
        // 如果当前设备已经关机，直接初始化相关变量
        sequencers_data_init();
    }

    user_sys_time_init(); // 初始化系统时间
    // 清空定时开关机计划；清空继电器的定时计划；这里跟第一次上电时，初始化的内容一致
    // 初始化设备每周定时开关机的计划时间表
    memset(&weekly_schedule, 0, sizeof(weekly_schedule_t)); 
    // 8个继电器、独立的每天激活、停用计划时间表
    memset(&weekly_schedule_relay, 0, sizeof(weekly_schedule_relay));

    return 0;
}
