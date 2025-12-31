#ifndef __USER_SCHEDULE_H__
#define __USER_SCHEDULE_H__ 

#include "includes.h"

#include "user_sys_time.h"
#include "relay_handle.h" // relay_index_t 

// 定义每天的开关机时间结构
typedef struct
{
    u8 enable;              // 该天是否启用定时开关机

    // power on time
    u8 on_hour; // 开机 小时 
    u8 on_minute; // 开机 分钟
    u8 on_second; // 开机 秒

    // power off time
    u8 off_hour; // 关机 小时
    u8 off_minute; // 关机 分钟
    u8 off_second; // 关机 秒
} daily_schedule_t;

// 定义一周的开关机计划
typedef struct
{
    daily_schedule_t schedule[7];  // 0=周日, 1=周一, ..., 6=周六
} weekly_schedule_t;

extern volatile weekly_schedule_t weekly_schedule; // 存放 时序器设备 的定时开关机的计划
extern volatile weekly_schedule_t weekly_schedule_relay[8]; // 存放 8个 继电器 的定时激活/定时停用计划

void weekly_schedule_relay_set(
    relay_index_t relay_index,
    const user_sys_time_t active_time,
    const user_sys_time_t deactive_time);
void week_schedule_relay_set_by_weekday(
    relay_index_t relay_index,
    u8 weekday,
    const user_sys_time_t active_time,
    const user_sys_time_t deactive_time);
void weekly_schedule_relay_cancel_by_weekday(relay_index_t relay_index, u8 weekday);


void weekly_schedule_info_set(const user_sys_time_t power_on_time, const user_sys_time_t power_off_time);
void weekly_schedule_info_cancel(u8 weekday);
void weekly_schedule_info_handle(void);
void weekly_schedule_relay_info_handle(void);

#endif