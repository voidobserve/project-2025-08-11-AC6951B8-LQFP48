#include "user_sys_time.h"
#include "alarm.h" 

/**
 * @brief 获取系统时间
 *      需要注意 传参类型  
 *       
 * @param time 
 * @return int 0：获取成功，1：获取失败
 */
int user_sys_time_get(user_sys_time_t* time)
{
    void* rtc_dev = dev_open("rtc", NULL);
    int ret  = 0;

    if (rtc_dev)
    {
        // 获取系统时间
        dev_ioctl(rtc_dev, IOCTL_GET_SYS_TIME, (u32 *)time);

        // 打印当前时间
        printf("Current time: %d-%02d-%02d %02d:%02d:%02d\n",
            time->year, time->month, time->day,
            time->hour, time->min, time->sec);

        time->weekday = rtc_calculate_week_val(time);
        printf("cur weekday: %d\n", time->weekday);

        dev_close(rtc_dev);
    }
    else
    {
        printf("rtc dev err\n");
        ret = 1;
    }

    return ret;
}

/**
 * @brief 设置系统时间
 *      需要注意 传参类型     
 * 
 * @param time 
 */
void user_sys_time_set(user_sys_time_t* time)
{  
    sys_time_t sys_time = {0};
    sys_time.year = time->year;
    sys_time.month = time->month;
    sys_time.day = time->day;
    sys_time.hour = time->hour;
    sys_time.min = time->min;
    sys_time.sec = time->sec;
    rtc_update_time_api(&sys_time);
}