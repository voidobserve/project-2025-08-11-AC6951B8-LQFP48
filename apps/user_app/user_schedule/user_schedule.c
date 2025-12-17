#include "user_schedule.h"
#include "syscfg_id.h"

#include "user_sys_time.h"

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
 * @brief 根据 一周的开关机计划时间表，进行定时开关机
 * 
 */
void weekly_schedule_info_handle(void)
{
    user_sys_time_t current_time = {0}; // 存放当前时间
    int ret = 0;
    ret =  user_sys_time_get(&current_time);
    if (0 != ret)
    {
        // 如果获取时间失败，退出函数，等下一次获取
        return;
    }

    // 判断当前时间是否与 计划时间表 的一致
    if (weekly_schedule.schedule[current_time.weekday].enable) // 当天有定时开关机计划
    {
        daily_schedule_t today = weekly_schedule.schedule[current_time.weekday];
        if (current_time.hour != today.on_hour)
        {
            return;
        }

    }


}
