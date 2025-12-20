#ifndef __USER_SCHEDULE_H__
#define __USER_SCHEDULE_H__ 

#include "includes.h"

// 定义每天的开关机时间结构
typedef struct
{
    u8 enable;              // 该天是否启用定时开关机
    u8 on_hour;             // 开机 小时
    u8 on_minute;           // 开机 分钟
    u8 on_second; // 开机 秒

    u8 off_hour;            // 关机 小时
    u8 off_minute; // 关机 分钟
    u8 off_second; // 关机 秒
} daily_schedule_t;

// 定义一周的开关机计划
typedef struct
{
    daily_schedule_t schedule[7];  // 0=周日, 1=周一, ..., 6=周六
} weekly_schedule_t;

void weekly_schedule_info_set(const user_sys_time_t power_on_time, const user_sys_time_t power_off_time);
void weekly_schedule_info_handle(void);

#endif