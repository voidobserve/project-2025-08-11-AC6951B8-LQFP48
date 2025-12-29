#include "user_schedule.h"
#include "syscfg_id.h"

#include "sys_time.h"
#include "user_sys_time.h" // 用户自定义的系统时间接口
#include "sequencer.h" 
#include "user_config.h"

#define FLASH_CRC_DATA 0xC5

volatile weekly_schedule_t weekly_schedule; // 存放定时开关机的计划
// 存放继电器定时激活/停用的计划
volatile weekly_schedule_t weekly_schedule_relay[8] = { 0 }; // 存放8个继电器的定时激活/定时停用计划

// void weekly_schedule_info_save(void)
// {

// }

// void weekly_schedule_info_read(void)
// {

// }


// void weekly_schedule_info_init(void)
// {

// }

/**
 * @brief 根据传入的参数，设置继电器的定时激活和定时停用时间
 *          目前是根据传参设置每一天的定时计划
 * @attention 注意：如果传参都为0，取消该继电器每一天的定时计划
 *
 * @param active_time
 * @param deactive_time
 * @return * void
 */
void weekly_schedule_relay_set(
    relay_index_t relay_index,
    const user_sys_time_t active_time,
    const user_sys_time_t deactive_time)
{
    for (u8 i = 0; i < 7; i++) // 遍历7天，星期0 ~ 星期6
    {
        weekly_schedule_relay[relay_index].schedule[i].on_hour = active_time.hour;
        weekly_schedule_relay[relay_index].schedule[i].on_minute = active_time.min;

        weekly_schedule_relay[relay_index].schedule[i].off_hour = deactive_time.hour;
        weekly_schedule_relay[relay_index].schedule[i].off_minute = deactive_time.min;

        if (active_time.hour == 0 &&
            active_time.min == 0 &&
            deactive_time.hour == 0 &&
            deactive_time.min == 0
            )
        {
            // 如果传入的时间全为0，不使能该定时计划
            weekly_schedule_relay[relay_index].schedule[i].enable = 0;
        }
        else
        {
            weekly_schedule_relay[relay_index].schedule[i].enable = 1;
        }
    }
}

// 根据存放好的继电器定时激活、停用计划，执行对应的激活、停用操作
void weekly_schedule_relay_info_handle(void)
{
    user_sys_time_t current_time = { 0 }; // 存放当前时间
    int ret = 0;

    if (sequencers.on_ff == DEVICE_OFF)
    {
        // 如果设备没有开机，不执行继电器的操作
        return;
    }

    ret = user_sys_time_get(&current_time); // 获取一次当前系统时间
    if (0 != ret)
    {
        // 如果获取时间失败，退出函数，等下一次获取
        return;
    }

    for (u8 i = 0; i < 7; i++) // 遍历8个继电器
    {  
        if (weekly_schedule_relay[i].schedule[0].enable == 0)
        {
            // 如果当前继电器没有定时计划，跳过当前循环
            continue;
        }

        // 检查是否到达开启时间
        if (weekly_schedule_relay[i].schedule[current_time.weekday].on_hour == current_time.hour &&
            weekly_schedule_relay[i].schedule[current_time.weekday].on_minute == current_time.min &&
            weekly_schedule_relay[i].schedule[current_time.weekday].on_second == current_time.sec &&
            sequencers.relay[i].cur_status_on_off == DEVICE_OFF)
        {
            // 如果到了对应的激活时间，并且继电器没有激活
            // 如果调用该函数的周期小于1s，那么时间到来的1s内会进入多次 
            
            sequencer_relay_status_setting(i, RELAY_STATUS_ACTIVE);
        }

        if (weekly_schedule_relay[i].schedule[current_time.weekday].off_hour == current_time.hour &&
            weekly_schedule_relay[i].schedule[current_time.weekday].off_minute == current_time.min &&
            weekly_schedule_relay[i].schedule[current_time.weekday].off_second == current_time.sec &&
            sequencers.relay[i].cur_status_on_off == DEVICE_ON) 
        {
            // 如果到了对应的停用时间，并且继电器没有停用
            // 如果调用该函数的周期小于1s，那么时间到来的1s内会进入多次 

            sequencer_relay_status_setting(i, RELAY_STATUS_DEACTIVE);
        }
    } 
}

/**
 * @brief 根据传入的时间，设置周的开关机计划
 *      注意：输入的参数，必须为 同一个 星期名称。
 *      例如 power_on_time 为星期一，power_off_time 也要是星期一
 *
 * @param power_on_time 计划开机时间
 * @param power_off_time 计划关机时间
 */
void weekly_schedule_info_set(const user_sys_time_t power_on_time, const user_sys_time_t power_off_time)
{
    if (power_on_time.weekday != power_off_time.weekday ||
        power_on_time.hour > 23 ||
        power_on_time.min > 59 ||
        power_on_time.sec > 59 ||
        power_off_time.hour > 23 ||
        power_off_time.min > 59 ||
        power_off_time.sec > 59
        )
    {
        /*
            如果传参不是同一个星期名称，或者传入的 时分秒 不合法，则退出函数
        */
        printf("[%s]: parameter invalled\n", __func__);
        return;
    }

    weekly_schedule.schedule[power_on_time.weekday].enable = 1;
    weekly_schedule.schedule[power_on_time.weekday].on_hour = power_on_time.hour;
    weekly_schedule.schedule[power_on_time.weekday].on_minute = power_on_time.min;
    weekly_schedule.schedule[power_on_time.weekday].on_second = power_on_time.sec;

    weekly_schedule.schedule[power_off_time.weekday].off_hour = power_off_time.hour;
    weekly_schedule.schedule[power_off_time.weekday].off_minute = power_off_time.min;
    weekly_schedule.schedule[power_off_time.weekday].off_second = power_off_time.sec;
}

/**
 * @brief 取消对应星期值的定时开关机计划
 *      注意：函数内部没有检测参数的合法性，传参前需要确保合法
 *
 * @param weekday 星期值
 * @return * void
 */
void weekly_schedule_info_cancel(u8 weekday)
{
    weekly_schedule.schedule[weekday].enable = 0;
}

/**
 * @brief 根据 一周的开关机计划时间表，进行定时开关机
 *
 */
void weekly_schedule_info_handle(void)
{
    user_sys_time_t current_time = { 0 }; // 存放当前时间
    int ret = 0;
    ret = user_sys_time_get(&current_time);
    if (0 != ret)
    {
        // 如果获取时间失败，退出函数，等下一次获取
        return;
    }

    // 判断当前时间是否与 计划时间表 的一致
    if (weekly_schedule.schedule[current_time.weekday].enable) // 当天有定时开关机计划
    {
        daily_schedule_t today = weekly_schedule.schedule[current_time.weekday];

        // 检查是否到达开机时间
        if (DEVICE_OFF == sequencers.on_ff && // 如果设备当前处于 关机状态
            current_time.hour == today.on_hour &&
            current_time.min == today.on_minute &&
            current_time.sec == today.on_second)
        {
            // 如果调用该函数的周期小于1s，那么时间到来的1s内会进入多次 
#if USER_DEBUG_ENABLE
            printf("power on time comes\n");
#endif

            if (DEVICE_ON == sequencers.on_ff)
            {
                // 如果已经完成开机，不执行开机操作
                return;
            }

            sequencer_power_on();
        }

        // 检查是否到达关机时间
        if (DEVICE_ON == sequencers.on_ff && // 如果设备当前处于 开机状态
            current_time.hour == today.off_hour &&
            current_time.min == today.off_minute &&
            current_time.sec == today.off_second)
        {
            // 如果调用该函数的周期小于1s，那么时间到来的1s内会进入多次
#if USER_DEBUG_ENABLE
            printf("power off time comes\n");
#endif

            if (DEVICE_OFF == sequencers.on_ff)
            {
                // 如果已经完成关机，不执行关机操作
                return;
            }

            sequencer_power_off();
        }
    }
}
