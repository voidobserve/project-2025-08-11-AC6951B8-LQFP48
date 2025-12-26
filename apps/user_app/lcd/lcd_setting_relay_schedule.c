#include "lcd_setting_relay_schedule.h"
#include "lcd1621.h"
#include "user_schedule.h"

volatile lcd_setting_relay_schedule_t lcd_setting_relay_active_schedule = { 0 };
volatile lcd_setting_relay_schedule_t lcd_setting_relay_deactive_schedule = { 0 };

// USER_TO_DO 构造函数
void lcd_setting_relay_active_constructor(void)
{
    lcd_setting_relay_active_schedule.blink_cnt = 0;
    lcd_setting_relay_active_schedule.blink_dir = 0;
    lcd_setting_relay_active_schedule.cur_setting_time_unit = TIME_UNIT_HOUR;
    lcd_setting_relay_active_schedule.timeout_cnt = 0;
    lcd_setting_relay_active_schedule.flag_is_timeout = 0;

}

void lcd_setting_relay_active_schedule_animation_fix(void)
{
    lcd_setting_relay_active_schedule.blink_cnt = 0;
    lcd_setting_relay_active_schedule.blink_dir = 0;
}

void lcd_setting_relay_active_shcedule_timeout_add_10ms(void)
{
    if (lcd_setting_relay_active_schedule.timeout_cnt < 65535)
    {
        // 防止越界
        lcd_setting_relay_active_schedule.timeout_cnt++;
    }
}

void lcd_setting_relay_active_schedule_animation(void)
{
    // user_schedule_t 
    // daily_schedule_t
    // weekly_schedule_t
}
