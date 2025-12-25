#ifndef __USER_SYS_TIME_H__
#define __USER_SYS_TIME_H__ 

#include "includes.h"

// 定义设置系统时间期间，当前正在设置的时间单位
typedef enum
{
	TIME_UNIT_YEAR = 0x00,
	TIME_UNIT_MONTH,
	TIME_UNIT_DAY,
	TIME_UNIT_HOUR,
	TIME_UNIT_MIN,
	TIME_UNIT_SEC,
} time_unit_t;


typedef struct
{
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
    u8 weekday; // 星期：0 ~ 6；  0：星期日，1：星期一
} user_sys_time_t;

extern volatile user_sys_time_t cur_setting_sys_time;

u8 is_user_time_valid(user_sys_time_t time);
u8 is_user_hour_valid(user_sys_time_t time);
u8 is_user_min_valid(user_sys_time_t time);
u8 is_user_sec_valid(user_sys_time_t time);

int user_sys_time_get(user_sys_time_t* time);
void user_sys_time_set(user_sys_time_t* time);

void user_sys_time_init(void);

#endif