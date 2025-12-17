#ifndef __USER_SYS_TIME_H__
#define __USER_SYS_TIME_H__ 

#include "includes.h"

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

int user_sys_time_get(user_sys_time_t* time);
void user_sys_time_set(user_sys_time_t* time);

#endif