#ifndef __LCD_SETTING_RELAY_SCHEDULE_H__
#define __LCD_SETTING_RELAY_SCHEDULE_H__ 

#include "includes.h"

#include "user_sys_time.h"
#include "user_schedule.h"
#include "relay_handle.h"

// 定义在设置继电器定时激活计划期间，lcd动画使用到的各个变量
typedef struct
{ 
	time_unit_t cur_setting_time_unit; // 当前正在设置的时间单位
	u16 blink_cnt; // 闪烁时间的计数值，控制每个一段时间闪烁一次要显示的、正在设置的系统时间
	u8 blink_dir; // 闪烁方向，0：常亮，1：熄灭（不显示） 

	u16 timeout_cnt; // 设置 时间 期间，超时时间的计数值
	u8 flag_is_timeout; // 设置 时间 期间，是否超时

	// 当前正在设置 激活时间 还是 停用时间
	u8 flag_is_setting_active_time;

	relay_index_t relay_index; // 当前正在设置的继电器索引
 
	daily_schedule_t schedule;  
} lcd_setting_relay_schedule_t;

void lcd_setting_relay_active_constructor(relay_index_t relay_index);

void lcd_setting_relay_time_param_add(void);
void lcd_setting_relay_time_param_sub(void);

void lcd_setting_relay_time_unit_switch_to_prev(void);
void lcd_setting_relay_time_unit_switch_to_next(void);

void lcd_setting_relay_time_get(user_sys_time_t* active_time, user_sys_time_t* deactive_time);
void lcd_setting_relay_schedule_get_index(relay_index_t * relay_index); // 
 
void lcd_setting_relay_schedule_animation_fix(void);
void lcd_setting_relay_schedule_timeout_reset(void);
void lcd_setting_relay_active_schedule_animation(void); 
void lcd_setting_relay_deactive_schedule_animation(void);

#endif 