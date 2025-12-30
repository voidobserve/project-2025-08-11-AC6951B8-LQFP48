#include "user_lcd_handle.h"
#include "lcd1621.h"
#include "sequencer.h"

#include "lcd_setting_relay_schedule.h"

volatile lcd_setting_sys_time_t lcd_setting_sys_time = { 0 };

// USER_TO_DO 构造函数
void lcd_setting_sys_time_constructor(void)
{

}

/**
 * @brief 在设置系统时间时，如果没有操作，累加超时时间
 *
 * @return * void
 */
void lcd_setting_sys_time_timeout_add_10ms(void)
{
	if (lcd_setting_sys_time.timeout_cnt < 65535)
	{
		// 防止越界
		lcd_setting_sys_time.timeout_cnt++;
	}
}

/**
 * @brief 在设置系统时间时，重置超时时间，清空超时标志位
 *
 * @return * void
 */
void lcd_setting_sys_time_timeout_reset(void)
{
	lcd_setting_sys_time.timeout_cnt = 0;
	lcd_setting_sys_time.flag_is_timeout = 0;
}

//
u8 lcd_setting_sys_time_is_timeout(void)
{
	// 如果超时时间计数是10ms计数一次，超时时间 == timeout_cnt * 10ms，以此类推
	if (lcd_setting_sys_time.timeout_cnt >= 1000)
	{
		lcd_setting_sys_time.flag_is_timeout = 1;
	}
	else
	{
		// 如果未超时
		lcd_setting_sys_time.flag_is_timeout = 0;
	}

	return lcd_setting_sys_time.flag_is_timeout;
}



/**
 * @brief 设置系统时间时，切换当前设置的时间单位
 *
 * @param time_unit
 */
void lcd_setting_sys_time_unit_change(time_unit_t time_unit)
{
	// lcd_setting_sys_time_unit = time_unit;

	lcd_setting_sys_time.cur_setting_time_unit = time_unit;
}

/**
 * @brief 设置系统时间时，获取正在设置时间单位（供外部函数调用）
 *
 * @param time_unit
 */
void lcd_setting_sys_time_unit_get(time_unit_t* time_unit)
{
	// *time_unit = lcd_setting_sys_time_unit;
	*time_unit = lcd_setting_sys_time.cur_setting_time_unit;
}

/**
 * @brief 设置系统时间时，切换当前设置的时间单位为下一个时间单位
 * 		例如：设置年份 -> 设置月份 -> 设置日期
 *
 */
void lcd_setting_sys_time_unit_switch_to_next(void)
{
	if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_YEAR)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_MONTH);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_MONTH)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_DAY);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_DAY)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_HOUR);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_HOUR)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_MIN);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_MIN)
	{
		// 不再向下切换
	}
}

void lcd_setting_sys_time_unit_switch_to_prev(void)
{
	if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_YEAR)
	{
		// 不再向上切换
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_MONTH)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_YEAR);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_DAY)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_MONTH);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_HOUR)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_DAY);
	}
	else if (lcd_setting_sys_time.cur_setting_time_unit == TIME_UNIT_MIN)
	{
		lcd_setting_sys_time_unit_change(TIME_UNIT_HOUR);
	}
}


/**
 * @brief lcd显示系统时间动画，会不让时间闪烁，固定显示一段时间（供外部函数调用）
 *
 */
void lcd_setting_sys_time_animation_fix(void)
{
	// lcd_setting_sys_time_blink_cnt = 0;
	// lcd_setting_sys_time_blink_dir = 0;

	lcd_setting_sys_time.blink_cnt = 0;
	lcd_setting_sys_time.blink_dir = 0;
}

/**
 * @brief 控制LCD显示系统时间的动画
 *
 */
void lcd_setting_sys_time_animation(void)
{
	// 如果正在设置系统时间
	// 让正在设置的时间闪烁，1 ： 年 ，2：月-日，3：时：分

	user_sys_time_t cur_setting_sys_time = { 0 };
	// 在进入该模式之前，这里的 setting_time 应该在进入前获取一次系统时间
	user_setting_time_get(&cur_setting_sys_time);

	lcd_setting_sys_time.blink_cnt++;
	if (lcd_setting_sys_time.blink_cnt >= 100) // 调用时间10ms，那么这里的时间单位就是10ms
	{
		lcd_setting_sys_time.blink_cnt = 0;
		lcd_setting_sys_time.blink_dir = !lcd_setting_sys_time.blink_dir;
	}

	// 清除第 4 ~ 7 位数码管显示的内容
	clean_num(4);
	clean_num(5);
	clean_num(6);
	clean_num(7);

	// 根据当前设置的时间单位，显示正在设置的时间
	// if (TIME_UNIT_YEAR == lcd_setting_sys_time_unit)
	if (TIME_UNIT_YEAR == lcd_setting_sys_time.cur_setting_time_unit)
	{
		// if (0 == lcd_setting_sys_time_blink_dir)
		if (0 == lcd_setting_sys_time.blink_dir)
		{
			// lcd_clear_year(); // 需要先清除再写入数据，否则会有数据残留
			lcd_update_year(cur_setting_sys_time.year);
		}
		else
		{
			lcd_clear_year();
		}
	}
	// else if (TIME_UNIT_MONTH == lcd_setting_sys_time_unit)
	else if (TIME_UNIT_MONTH == lcd_setting_sys_time.cur_setting_time_unit)
	{
		// if (0 == lcd_setting_sys_time_blink_dir)
		if (0 == lcd_setting_sys_time.blink_dir)
		{
			// lcd_clear_month();
			lcd_update_month(cur_setting_sys_time.month);
		}
		else
		{
			lcd_clear_month();
		}

		// 月份闪烁，但是日期保持不变
		// lcd_clear_day();
		lcd_update_day(cur_setting_sys_time.day);
	}
	// else if (TIME_UNIT_DAY == lcd_setting_sys_time_unit)
	else if (TIME_UNIT_DAY == lcd_setting_sys_time.cur_setting_time_unit)
	{
		// if (0 == lcd_setting_sys_time_blink_dir)
		if (0 == lcd_setting_sys_time.blink_dir)
		{
			// lcd_clear_day();
			lcd_update_day(cur_setting_sys_time.day);
		}
		else
		{
			lcd_clear_day();
		}

		// 日期闪烁，但是月份保持不变
		// lcd_clear_month();
		lcd_update_month(cur_setting_sys_time.month);
	}
	// else if (TIME_UNIT_HOUR == lcd_setting_sys_time_unit)
	else if (TIME_UNIT_HOUR == lcd_setting_sys_time.cur_setting_time_unit)
	{
		// if (0 == lcd_setting_sys_time_blink_dir)
		if (0 == lcd_setting_sys_time.blink_dir)
		{
			// lcd_clear_hour();
			lcd_update_hour(cur_setting_sys_time.hour);
		}
		else
		{
			lcd_clear_hour();
		}

		// 小时闪烁，但是分钟保持不变
		// lcd_clear_min();
		lcd_update_min(cur_setting_sys_time.min);
	}
	// else if (TIME_UNIT_MIN == lcd_setting_sys_time_unit)
	else if (TIME_UNIT_MIN == lcd_setting_sys_time.cur_setting_time_unit)
	{
		// if (0 == lcd_setting_sys_time_blink_dir)
		if (0 == lcd_setting_sys_time.blink_dir)
		{
			// lcd_clear_min();
			lcd_update_min(cur_setting_sys_time.min);
		}
		else
		{
			lcd_clear_min();
		}

		// 分钟闪烁，但是小时保持不变
		// lcd_clear_hour();
		lcd_update_hour(cur_setting_sys_time.hour);
	}
	// else if (TIME_UNIT_SEC == lcd_setting_sys_time_unit)
	// else if (TIME_UNIT_SEC == lcd_setting_sys_time.cur_setting_time_unit)
	// {

	// }
}

//  LCD屏显示处理  10ms执行一次
void lcdseg_handle(void)
{
#if 0 // 测试显示屏的功能是否正常
	lcd_open_frame(); // 测试用 -- 打开背光，显示边框
	check_lcd_display(); // 测试用 
	lcd1621_write_data(dis_data, 16);
	// printf("test \n");
#endif

#if 1
	if (sequencer_status == SEQUENCER_STATUS_NONE)
	{
		// 上电期间，一直显示LCD，显示交流电电压：
		clean_num(1);clean_num(2);clean_num(3);   // 清除第一位~第三位数码管显示的内容
		clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏
		clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4)); // 不显示： ' " 
		clean_dis(clrbit(SEG_S6)); // 关闭"W"
		// // 交流电电压： 
		lcd_update_ac_voltage();
		make_dis(SEG_S5); // lcd显示符号：V		

		// 后面需要显示年份xx时间，再切换到显示月日xx时间，最后切换到显示时分 
		{
			static u8 cnt = 0;
			static u8 dir = 0; // 控制当前要显示的时间单位，0：年，1：月-日，2：时：分
			static volatile user_sys_time_t user_sys_time = {0};

			cnt++;
			if (cnt >= 200)
			{
				cnt = 0;

				dir++;
				if (dir >= 3)
				{
					dir = 0;
				}
			}

			user_sys_time_get(&user_sys_time);
			if (dir == 0)
			{
				lcd_clear_year();
				lcd_update_year(user_sys_time.year);
			}
			else if (dir == 1)
			{
				lcd_clear_month();
				lcd_update_month(user_sys_time.month);

				lcd_clear_day();
				lcd_update_day(user_sys_time.day);
			}
			else if (dir == 2)
			{
				lcd_clear_hour();
				lcd_update_hour(user_sys_time.hour);

				lcd_clear_min();
				lcd_update_min(user_sys_time.min);
			}
		}
	}
	else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
	{
		lcd_clear_ac_voltage(); // 清除数码管显示的交流电压数据
		lcd_update_ac_voltage();
		lcd_setting_sys_time_animation();
		make_dis(SEG_S5); // lcd显示符号：V		

		lcd_setting_sys_time_timeout_add_10ms();
		if (lcd_setting_sys_time_is_timeout())
		{
			// 如果 设置系统时间 超时，返回普通模式
			sequencer_status = SEQUENCER_STATUS_NONE;
			lcd_setting_sys_time_timeout_reset();

			// 保存设置的时间
			user_sys_time_t user_sys_time = { 0 };
			user_setting_time_get(&user_sys_time);
			user_sys_time_set(&user_sys_time);
			// 保存用户数据
			os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
		}
	}
	else if (sequencer_status == SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE)
	{
		// 如果是设置单个继电器的激活时间计划
		clean_dis(clrbit(SEG_S5)); // 不显示符号：V
		clean_num(1); // 清除第1位数码管显示的数据
		clean_num(2);
		clean_num(3);
		clean_num(4);
		clean_num(5);
		clean_num(6);
		clean_num(7);

		lcd_show_alphabet_in_seg_1(0); // 第一位数码管显示 P ，表示当前正在设置继电器的定时激活时间

		lcd_setting_relay_active_schedule_animation();
		lcd_setting_relay_shcedule_timeout_add_10ms();
		if (lcd_setting_relay_schedule_is_timeout())
		{
			// 如果设置 继电器的定时激活时间 超时
			relay_index_t relay_index = RELAY_INDEX_INVALID; // 存放继电器的索引
			user_sys_time_t active_time = { 0 }; // 存放定时激活时间
			user_sys_time_t deactive_time = { 0 }; // 存放定时停用时间
			lcd_setting_relay_schedule_get_index(&relay_index); // 获取当前正在设置的继电器索引
			lcd_setting_relay_time_get(&active_time, &deactive_time);
			weekly_schedule_relay_set(relay_index, active_time, deactive_time);

			sequencer_status = SEQUENCER_STATUS_NONE;
			// 保存用户数据
			os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
		}
	}
	else if (sequencer_status == SEQUENCER_STATUS_SETTING_RELAY_DEACTIVE_SCHEDULE)
	{
		// 如果是设置单个继电器的 禁用时间 计划
		clean_dis(clrbit(SEG_S5)); // 不显示符号：V
		clean_num(1); // 清除第1位数码管显示的数据
		clean_num(2);
		clean_num(3);
		clean_num(4);
		clean_num(5);
		clean_num(6);
		clean_num(7);

		lcd_show_alphabet_in_seg_1(1); // 第一位数码管显示 F ，表示当前正在设置继电器的定时停用时间

		lcd_setting_relay_deactive_schedule_animation();
		lcd_setting_relay_shcedule_timeout_add_10ms();
		if (lcd_setting_relay_schedule_is_timeout())
		{
			// 如果设置 继电器的定时禁用时间 超时 
			relay_index_t relay_index = RELAY_INDEX_INVALID; // 存放继电器的索引
			user_sys_time_t active_time = { 0 }; // 存放定时激活时间
			user_sys_time_t deactive_time = { 0 }; // 存放定时停用时间
			lcd_setting_relay_schedule_get_index(&relay_index); // 获取当前正在设置的继电器索引
			lcd_setting_relay_time_get(&active_time, &deactive_time);
			weekly_schedule_relay_set(relay_index, active_time, deactive_time);

			sequencer_status = SEQUENCER_STATUS_NONE;
			// 保存用户数据
			os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
		}
	}

	lcd1621_write_data(dis_data, 16); // 将lcd显示缓冲区的数据发送给屏幕驱动ic 

#endif
}
