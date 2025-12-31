#include "user_sys_time.h"
#include "alarm.h" 
#include "user_lcd_handle.h"

// 注意：RTC的时间掉电会丢失

// 存放每个月有多少天
static const u8 month_day_table[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
    存放当前要设置的系统时间，
    进入设置系统时间前，要先获得当前时间，作为 cur_setting_sys_time 的值
*/
static volatile user_sys_time_t cur_setting_sys_time = { 0 };

int is_leap_year(u16 year)
{
    // 能被4整除且不能被100整除，或者能被400整除
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
    {
        return 1;
    }

    return 0;
}

/**
 * @brief 判断传入的时间是否合法
 * 
 * @param time 
 * @return * int 
 */
int user_time_is_valid(user_sys_time_t time)
{
    // 判断年份是否为闰年
    u8 flag_is_leap_year = 0;


    if (time.year < 2000 || time.year > 2099)
    {
        // 年份超出范围，返回0
        return 0;
    }

    if (time.month < 1 || time.month > 12)
    {
        // 月份超出范围，返回0
        return 0;
    }

    // 能被4整除且不能被100整除，或者能被400整除
    // if ((time.year % 4 == 0 && time.year % 100 != 0) || (time.year % 400 == 0))
    // {
    //     flag_is_leap_year = 1;
    // }
    flag_is_leap_year = is_leap_year(time.year);

    if (time.day < 1)
    {
        return 0;
    }

    // 如果是闰年，2月份有29天，如果不是，2月份28天
    if (time.month == 2)
    {
        if (flag_is_leap_year)
        {
            if (time.day > 29)
            {
                return 0;
            }
        }
        else
        {
            if (time.day > 28)
            {
                return 0;
            }
        }
    }
    else
    {
        if (time.day > month_day_table[time.month - 1])
        {
            // 如果天数超出了当月的天数，不合法
            return 0;
        }
    }

    if (time.hour > 23)
    {
        return 0;
    }

    if (time.min > 59)
    {
        return 0;
    }

    if (time.sec > 59)
    {
        return 0;
    }

    return 1;
}


/**
 * @brief 设置时间时，递增当前的年份（限制最大值为2099）
 *
 */
void user_setting_time_year_add(void)
{
    if (cur_setting_sys_time.year < 2099)
    {
        cur_setting_sys_time.year++;
    }
}

/**
 * @brief 设置时间时，递减当前的年份（限制最小值为2000）
 *
 */
void user_setting_time_year_sub(void)
{
    if (cur_setting_sys_time.year > 2000)
    {
        cur_setting_sys_time.year--;
    }
    else
    {
        cur_setting_sys_time.year = 2000;
    }
}

void user_setting_time_month_add(void)
{
    if (cur_setting_sys_time.month < 12)
    {
        cur_setting_sys_time.month++;
    }
}

void user_setting_time_month_sub(void)
{
    if (cur_setting_sys_time.month > 1)
    {
        cur_setting_sys_time.month--;
    }
    else
    {
        cur_setting_sys_time.month = 1;
    }
}

void user_setting_time_day_add(void)
{
    // 日期要根据年份、月份来限制最大值
    if (cur_setting_sys_time.day < 31)
    {
        cur_setting_sys_time.day++;
    }
}

void user_setting_time_day_sub(void)
{
    if (cur_setting_sys_time.day > 1)
    {
        cur_setting_sys_time.day--;
    }
    else
    {
        cur_setting_sys_time.day = 1;
    }
}

void user_setting_time_hour_add(void)
{
    if (cur_setting_sys_time.hour < 23)
    {
        cur_setting_sys_time.hour++;
    }
}

void user_setting_time_hour_sub(void)
{
    if (cur_setting_sys_time.hour > 0)
    {
        cur_setting_sys_time.hour--;
    }
    else
    {
        cur_setting_sys_time.hour = 0;
    }
}

void user_setting_time_min_add(void)
{
    if (cur_setting_sys_time.min < 59)
    {
        cur_setting_sys_time.min++;
    }
}

void user_setting_time_min_sub(void)
{
    if (cur_setting_sys_time.min > 0)
    {
        cur_setting_sys_time.min--;
    }
    else
    {
        cur_setting_sys_time.min = 0;
    }
}

// void user_setting_time_sec_add(void)
// {
//     if (cur_setting_sys_time.sec < 59)
//     {
//         cur_setting_sys_time.sec++;
//     }
// }

void user_setting_time_check_up(void)
{
    if (!user_time_is_valid(cur_setting_sys_time))
    {
        // 如果修改时间后发现时间不合法
        if (cur_setting_sys_time.year < 2000)
        {
            cur_setting_sys_time.year = 2000;
        }
        else if (cur_setting_sys_time.year > 2099)
        {
            cur_setting_sys_time.year = 2099;
        }

        if (cur_setting_sys_time.month < 1)
        {
            cur_setting_sys_time.month = 1;
        }
        else if (cur_setting_sys_time.month > 12)
        {
            cur_setting_sys_time.month = 12;
        }

        if (cur_setting_sys_time.day < 1)
        {
            cur_setting_sys_time.day = 1;
        }
        else
        {
            if (cur_setting_sys_time.month == 2)
            {
                u8 flag_is_leap_year = is_leap_year(cur_setting_sys_time.year);
                if (flag_is_leap_year)
                {
                    if (cur_setting_sys_time.day > 29)
                    {
                        cur_setting_sys_time.day = 29;
                    }
                }
                else
                {
                    if (cur_setting_sys_time.day > 28)
                    {
                        cur_setting_sys_time.day = 28;
                    }
                }
            }
            else
            {
                if (cur_setting_sys_time.day > month_day_table[cur_setting_sys_time.month - 1])
                {
                    cur_setting_sys_time.day = month_day_table[cur_setting_sys_time.month - 1];
                }
            }
        }

        if (cur_setting_sys_time.hour > 23)
        {
            cur_setting_sys_time.hour = 23;
        }

        if (cur_setting_sys_time.min > 59)
        {
            cur_setting_sys_time.min = 59;
        }

        if (cur_setting_sys_time.sec > 59)
        {
            cur_setting_sys_time.sec = 59;
        }
    }
}


/**
 * @brief 设置时间时，递增 当前设置的参数
 *
 */
void user_setting_time_param_add(void)
{
    time_unit_t cur_setting_time_unit;
    lcd_setting_sys_time_unit_get(&cur_setting_time_unit);
    if (cur_setting_time_unit == TIME_UNIT_YEAR)
    {
        user_setting_time_year_add();
    }
    else if (cur_setting_time_unit == TIME_UNIT_MONTH)
    {
        user_setting_time_month_add();
    }
    else if (cur_setting_time_unit == TIME_UNIT_DAY)
    {
        user_setting_time_day_add();
    }
    else if (cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        user_setting_time_hour_add();
    }
    else if (cur_setting_time_unit == TIME_UNIT_MIN)
    {
        user_setting_time_min_add();
    }

    user_setting_time_check_up();
}

/**
 * @brief 设置时间时，递减 当前设置的参数
 *
 */
void user_setting_time_param_sub(void)
{
    time_unit_t cur_setting_time_unit;
    lcd_setting_sys_time_unit_get(&cur_setting_time_unit);
    if (cur_setting_time_unit == TIME_UNIT_YEAR)
    {
        user_setting_time_year_sub();
    }
    else if (cur_setting_time_unit == TIME_UNIT_MONTH)
    {
        user_setting_time_month_sub();
    }
    else if (cur_setting_time_unit == TIME_UNIT_DAY)
    {
        user_setting_time_day_sub();
    }
    else if (cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        user_setting_time_hour_sub();
    }
    else if (cur_setting_time_unit == TIME_UNIT_MIN)
    {
        user_setting_time_min_sub();
    }

    user_setting_time_check_up();
}

void user_setting_time_get(user_sys_time_t* time)
{
    time->year = cur_setting_sys_time.year;
    time->month = cur_setting_sys_time.month;
    time->day = cur_setting_sys_time.day;
    time->hour = cur_setting_sys_time.hour;
    time->min = cur_setting_sys_time.min;
    // time->sec = cur_setting_sys_time.sec; // 当前设置的没有秒这一单位
}

void user_setting_time_set(user_sys_time_t* time)
{
    cur_setting_sys_time.year = time->year;
    cur_setting_sys_time.month = time->month;
    cur_setting_sys_time.day = time->day;

    cur_setting_sys_time.hour = time->hour;
    cur_setting_sys_time.min = time->min;
    // cur_setting_sys_time.sec = time->sec; // 当前设置的没有秒这一单位
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
#if USER_DEBUG_ENABLE
        // printf("Current time: %d-%02d-%02d %02d:%02d:%02d\n",
        //     time->year, time->month, time->day,
        //     time->hour, time->min, time->sec);
#endif

        time->weekday = rtc_calculate_week_val(time);
#if USER_DEBUG_ENABLE
        printf("cur weekday: %d\n", time->weekday);
#endif

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

void user_sys_time_init(void)
{
    // 2000-01-01 00:00:00
    volatile user_sys_time_t time = { 2000, 01, 01 , 00 , 00, 00 };
    user_sys_time_set(&time);
}
