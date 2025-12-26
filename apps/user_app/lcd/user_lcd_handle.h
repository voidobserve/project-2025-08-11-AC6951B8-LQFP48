#ifndef __USER_LCD_HANDLE_H__
#define __USER_LCD_HANDLE_H__ 

#include "includes.h"

#include "user_sys_time.h"

// 定义在设置系统时间期间，lcd动画使用到的各个变量
typedef struct
{
	// user_sys_time_t cur_setting_sys_time; // 当前正在设置的系统时间
	time_unit_t cur_setting_time_unit; // 当前正在设置的时间单位
	u16 blink_cnt; // 闪烁时间的计数值，控制每个一段时间闪烁一次要显示的、正在设置的系统时间
	u8 blink_dir; // 闪烁方向，0：常亮，1：熄灭（不显示） 

	u16 timeout_cnt; // 设置系统时间期间，超时时间的计数值
	u8 flag_is_timeout; // 设置系统时间期间，是否超时

	// void (*time_out_add_10ms)(void);
	// void (*time_out_clear)(void);
	// u16 (*time_out_get)(void);
} lcd_setting_sys_time_t; // 将变量改成使用该结构体类型的变量



void lcd_setting_sys_time_timeout_add_10ms(void);
void lcd_setting_sys_time_timeout_reset(void);
u8 lcd_setting_sys_time_is_timeout(void);

void lcd_setting_sys_time_animation_fix(void);
void lcd_setting_sys_time_animation(void);
void lcd_setting_sys_time_unit_change(time_unit_t time_unit);
void lcd_setting_sys_time_unit_get(time_unit_t* time_unit);
void lcd_setting_sys_time_unit_switch_to_next(void);
void lcd_setting_sys_time_unit_switch_to_prev(void);
void lcd_setting_sys_time_animation(void);

#endif