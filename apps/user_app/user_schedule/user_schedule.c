#include "user_schedule.h"
#include "syscfg_id.h"

#include "sys_time.h"
#include "user_sys_time.h" // 用户自定义的系统时间接口
#include "sequencer.h" 

#define FLASH_CRC_DATA 0xC5

volatile weekly_schedule_t weekly_schedule;

void weekly_schedule_info_save(void)
{

}

void weekly_schedule_info_read(void)
{

}


void weekly_schedule_info_init(void)
{

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
            printf("power on time comes\n");
            sequencer_power_on();
        }

        // 检查是否到达关机时间
        if (DEVICE_ON == sequencers.on_ff && // 如果设备当前处于 开机状态
            current_time.hour == today.off_hour &&
            current_time.min == today.off_minute &&
            current_time.sec == today.off_second)
        {
            printf("power off time comes\n");
            sequencer_power_off();
        }
    }
}
