#ifndef __LCD_SETTING_RELAY_SCHEDULE_H__
#define __LCD_SETTING_RELAY_SCHEDULE_H__ 

#include "includes.h"

#include "user_sys_time.h"

// 定义在设置继电器定时激活计划期间，lcd动画使用到的各个变量
typedef struct
{
	time_unit_t cur_setting_time_unit; // 当前正在设置的时间单位
	u16 blink_cnt; // 闪烁时间的计数值，控制每个一段时间闪烁一次要显示的、正在设置的系统时间
	u8 blink_dir; // 闪烁方向，0：常亮，1：熄灭（不显示） 

	u16 timeout_cnt; // 设置 时间 期间，超时时间的计数值
	u8 flag_is_timeout; // 设置 时间 期间，是否超时

    void (*amimation)(void);
	
} lcd_setting_relay_schedule_t;

#endif