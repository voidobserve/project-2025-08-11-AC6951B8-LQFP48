#include "user_sys_time.h"
#include "alarm.h" 

/**
 * @brief 判断传入的时间是否合法（没有超出时间范围）
 *      注意：判断的时间不包括星期值
 * 
 * @param time 
 * @return u8 1：合法，0：不合法
 */
u8 is_user_time_valid(user_sys_time_t time)
{
    // 判断时间是否合法
    if (time.year < 2000 || time.year > 2099 || // 超出了 2000 ~ 2099 的范围
        time.month < 1 || time.month > 12 || // 超出了 1 ~ 12 的范围
        time.day < 1 || time.day > 31 || // 超出了 1 ~ 31 的范围
        time.hour > 23 || // 超出了 0 ~ 23 的范围
        time.min > 59 || // 超出了 0 ~ 59 的范围
        time.sec > 59 // 超出了 0 ~ 59 的范围
        )
    {
        return 0;
    }

    return 1;
}


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
    int ret = 0;

    if (rtc_dev)
    {
        // 获取系统时间
        dev_ioctl(rtc_dev, IOCTL_GET_SYS_TIME, (u32*)time);

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
    sys_time_t sys_time = { 0 };
    sys_time.year = time->year;
    sys_time.month = time->month;
    sys_time.day = time->day;
    sys_time.hour = time->hour;
    sys_time.min = time->min;
    sys_time.sec = time->sec;
    rtc_update_time_api(&sys_time);
}