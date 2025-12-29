#include "lcd_setting_relay_schedule.h"
#include "lcd1621.h"
#include "user_schedule.h"
#include "relay_handle.h" 
#include "sequencer.h"

static volatile lcd_setting_relay_schedule_t* __this = NULL;
volatile lcd_setting_relay_schedule_t lcd_setting_relay_schedule = { 0 };

// 构造函数
void lcd_setting_relay_active_constructor(relay_index_t relay_index)
{
    lcd_setting_relay_schedule.relay_index = relay_index;
    lcd_setting_relay_schedule.blink_cnt = 0;
    lcd_setting_relay_schedule.blink_dir = 0;
    lcd_setting_relay_schedule.cur_setting_time_unit = TIME_UNIT_HOUR;
    lcd_setting_relay_schedule.timeout_cnt = 0;
    lcd_setting_relay_schedule.flag_is_timeout = 0;
    lcd_setting_relay_schedule.flag_is_setting_active_time = 1; // 表示当前正在设置 激活时间

    // 找到对应的继电器，任意一天的计划（目前设置是每周的计划，所以可以找任意一天）
    if (weekly_schedule_relay[relay_index].schedule[0].enable == 1)
    {
        // 如果有使能定时激活/定时停用，初始化对应的变量
        memcpy(&lcd_setting_relay_schedule.schedule,
            &weekly_schedule_relay[relay_index].schedule[0],
            sizeof(daily_schedule_t));
    }
    else
    {
        // 如果没有计划，初始化数据
        memset(&lcd_setting_relay_schedule.schedule, 0, sizeof(daily_schedule_t));
    }

    __this = &lcd_setting_relay_schedule;
}

/**
 * @brief 在设置指定继电器定时激活、定时停用计划期间，获取当前继电器的索引值
 * 
 * @param relay_index 存放得到的继电器的索引值
 */
void lcd_setting_relay_schedule_get_index(relay_index_t * relay_index)
{
    *relay_index = __this->relay_index;
}

/**
 * @brief lcd显示 设置继电器定时时间动画，会不让时间闪烁，固定显示一段时间（供外部函数调用）
 *
 */
void lcd_setting_relay_schedule_animation_fix(void)
{
    __this->blink_cnt = 0;
    __this->blink_dir = 0;
}

/**
 * @brief 在设置继电器定时计划期间，重置超时计数，清空超时标志位
 *
 */
void lcd_setting_relay_schedule_timeout_reset(void)
{
    __this->timeout_cnt = 0; // 清空超时计数值
    __this->flag_is_timeout = 0; // 清空超时标志位
}

/**
 * @brief 在设置定时时间时，如果没有操作，累加超时时间
 *
 */
void lcd_setting_relay_active_shcedule_timeout_add_10ms(void)
{
    if (__this->timeout_cnt < 65535)
    {
        // 防止越界
        __this->timeout_cnt++;
    }
}

void lcd_setting_relay_time_hour_add(void)
{
    if (__this->flag_is_setting_active_time)
    {
        if (__this->schedule.on_hour < 23)
        {
            __this->schedule.on_hour++;
        }
    }
    else
    {
        if (__this->schedule.off_hour < 23)
        {
            __this->schedule.off_hour++;
        }
    }
}

void lcd_setting_relay_time_hour_sub(void)
{
    if (__this->flag_is_setting_active_time)
    {
        if (__this->schedule.on_hour > 0)
        {
            __this->schedule.on_hour--;
        }
        else
        {
            __this->schedule.on_hour = 0;
        }
    }
    else
    {
        if (__this->schedule.off_hour > 0)
        {
            __this->schedule.off_hour--;
        }
        else
        {
            __this->schedule.off_hour = 0;
        }
    }
}

void lcd_setting_relay_time_min_add(void)
{
    if (__this->flag_is_setting_active_time)
    {
        if (__this->schedule.on_minute < 59)
        {
            __this->schedule.on_minute++;
        }
    }
    else
    {
        if (__this->schedule.off_minute < 59)
        {
            __this->schedule.off_minute++;
        }
    }
}

void lcd_setting_relay_time_min_sub(void)
{
    if (__this->flag_is_setting_active_time)
    {
        if (__this->schedule.on_minute > 0)
        {
            __this->schedule.on_minute--;
        }
        else
        {
            __this->schedule.on_minute = 0;
        }
    }
    else
    {
        if (__this->schedule.off_minute > 0)
        {
            __this->schedule.off_minute--;
        }
        else
        {
            __this->schedule.off_minute = 0;
        }
    }
}

void lcd_setting_relay_time_param_add(void)
{
    if (__this->cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        lcd_setting_relay_time_hour_add();
    }
    else if (__this->cur_setting_time_unit == TIME_UNIT_MIN)
    {
        lcd_setting_relay_time_min_add();
    }
}

void lcd_setting_relay_time_param_sub(void)
{
    if (__this->cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        lcd_setting_relay_time_hour_sub();
    }
    else if (__this->cur_setting_time_unit == TIME_UNIT_MIN)
    {
        lcd_setting_relay_time_min_sub();
    }
}

// 设置定时时间时，切换当前的单位至 上一种
void lcd_setting_relay_time_unit_switch_to_prev(void)
{
    if (__this->cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        if (!__this->flag_is_setting_active_time)
        {
            // 如果当前设置的是 禁用时间，则切换到 激活时间
            sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
            __this->flag_is_setting_active_time = 1; // 更新对应的标志位

            // 切换到 激活时间的 分钟
            __this->cur_setting_time_unit = TIME_UNIT_MIN;
        }

        // 如果当前设置的是 激活时间，则不再向上切换 时间单位
    }
    else if (__this->cur_setting_time_unit == TIME_UNIT_MIN)
    {
        // 如果当前设置的是 分钟，则切换到设置 小时
        __this->cur_setting_time_unit = TIME_UNIT_HOUR;
    }
}

// 设置定时时间时，切换当前的单位至 下一种
void lcd_setting_relay_time_unit_switch_to_next(void)
{
    if (__this->cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        // 如果当前设置的是 小时，则切换到设置 分钟
        __this->cur_setting_time_unit = TIME_UNIT_MIN;
    }
    else if (__this->cur_setting_time_unit == TIME_UNIT_MIN)
    {
        if (__this->flag_is_setting_active_time)
        {
            // 如果当前设置的是 激活时间，则切换到设置 禁用时间
            sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_DEACTIVE_SCHEDULE;
            __this->flag_is_setting_active_time = 0;// 更新对应的标志位

            // 切换到 禁用时间的 小时
            __this->cur_setting_time_unit = TIME_UNIT_HOUR;
        }

        // 如果当前设置的是 禁用时间，则不再向下切换 时间单位
    }
}

/**
 * @brief 设置 继电器定时计划时，获取当前设置继电器对应的的定时计划
 * @attention 注意：目前只有 时 和 分 有效，每天x时x分激活，x时x分停用
 *
 * @param active_time
 * @param deactive_time
 * @return * void
 */
void lcd_setting_relay_time_get(user_sys_time_t* active_time, user_sys_time_t* deactive_time)
{
    active_time->hour = __this->schedule.on_hour;
    active_time->min = __this->schedule.on_minute;

    deactive_time->hour = __this->schedule.off_hour;
    deactive_time->min = __this->schedule.off_minute;
}

/**
 * @brief 控制LCD显示继电器的定时时间动画：设置激活时间
 *
 */
void lcd_setting_relay_active_schedule_animation(void)
{
    // 清除第 4 ~ 7 位数码管显示的内容
    clean_num(4);
    clean_num(5);
    clean_num(6);
    clean_num(7);


    make_num(2, 0); // 第二位七段数码管显示 0
    make_num(3, __this->relay_index + 1); // 第三位七段数码管显示 继电器编号

    // 控制数码管闪烁时间：
    __this->blink_cnt++;
    if (__this->blink_cnt >= 100) // 时间单位与调用该函数的周期有关，如果该函数10ms调用一次，这里就每10ms加一
    {
        __this->blink_cnt = 0;
        __this->blink_dir = !__this->blink_dir;
    }

    if (__this->cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        if (0 == __this->blink_dir)
        {
            lcd_update_hour(__this->schedule.on_hour);
        }
        else
        {
            lcd_clear_hour();
        }

        lcd_update_min(__this->schedule.on_minute);
    }
    else if (__this->cur_setting_time_unit == TIME_UNIT_MIN)
    {
        if (0 == __this->blink_dir)
        {
            lcd_update_min(__this->schedule.on_minute);
        }
        else
        {
            lcd_clear_min();
        }

        lcd_update_hour(__this->schedule.on_hour);
    }
}

/**
 * @brief 控制LCD显示继电器的定时时间动画：设置 禁用 时间
 *
 */
void lcd_setting_relay_deactive_schedule_animation(void)
{
    // 清除第 4 ~ 7 位数码管显示的内容
    clean_num(4);
    clean_num(5);
    clean_num(6);
    clean_num(7);

    make_num(2, 0); // 第二位七段数码管显示 0
    make_num(3, __this->relay_index + 1); // 第三位七段数码管显示 继电器编号

    // 控制数码管闪烁时间：
    __this->blink_cnt++;
    if (__this->blink_cnt >= 100) // 时间单位与调用该函数的周期有关，如果该函数10ms调用一次，这里就每10ms加一
    {
        __this->blink_cnt = 0;
        __this->blink_dir = !__this->blink_dir;
    }

    if (__this->cur_setting_time_unit == TIME_UNIT_HOUR)
    {
        if (0 == __this->blink_dir)
        {
            lcd_update_hour(__this->schedule.off_hour);
        }
        else
        {
            lcd_clear_hour();
        }

        lcd_update_min(__this->schedule.off_minute);
    }
    else if (__this->cur_setting_time_unit == TIME_UNIT_MIN)
    {
        if (0 == __this->blink_dir)
        {
            lcd_update_min(__this->schedule.off_minute);
        }
        else
        {
            lcd_clear_min();
        }

        lcd_update_hour(__this->schedule.off_hour);
    }
}
