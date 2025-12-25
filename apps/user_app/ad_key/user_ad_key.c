#include "user_ad_key.h"
#include "key_event_deal.h"
#include "relay_handle.h"
#include "sequencer.h"
#include "lcd1621.h"
#include "user_sys_time.h"

/**
 * @brief AD按键控制16路继电器
 *
 * @param keyevent    AD按键消息
 */
void ad_key_event_handle(int keyevent)
{
    // printf("ad key event == %d\n", keyevent);

    switch (keyevent)
    {
    case KEY0_AD_CLICK: // 第一路对应的继电器按键 

        // ===================================================================
    case KEY0_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_0);
    }
    break;
    // ===================================================================
    case KEY1_AD_CLICK:
    case KEY1_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_1);
    }
    break;

    // ===================================================================
    case KEY2_AD_CLICK:
    case KEY2_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_2);
    }
    break;
    // ===================================================================
    case KEY3_AD_CLICK:
    case KEY3_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_3);
    }
    break;
    // ===================================================================
    case KEY4_AD_CLICK:
    {
        if (sequencer_status == SEQUENCER_STATUS_NONE)
        {
            // 普通模式，翻转继电器的状态 
            if (DEVICE_ON == sequencers.on_ff && 0 == is_sequencer_in_delay())
            {
                // 如果没有开机，或者设备正在开关机的延时中，不处理该按键事件
                return;
            }


            sequencer_relay_status_toggle(RELAY_INDEX_4);
        }
        else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
        {
            // 如果正在设置系统时间
            // 递减当前调节的时间 
            time_unit_t cur_setting_time_unit = 0;
            lcd_setting_sys_time_unit_get(&cur_setting_time_unit);
            lcd_setting_sys_time_animation_fix();

            if (cur_setting_time_unit == TIME_UNIT_YEAR)
            {
                cur_setting_sys_time.year--;
            }
            else if (cur_setting_time_unit == TIME_UNIT_MONTH)
            {
                cur_setting_sys_time.month--;
            }
            else if (cur_setting_time_unit == TIME_UNIT_DAY)
            {
                cur_setting_sys_time.day--;
            }
            else if (cur_setting_time_unit == TIME_UNIT_HOUR)
            {
                cur_setting_sys_time.hour--;
            }
            else if (cur_setting_time_unit == TIME_UNIT_MIN)
            {
                cur_setting_sys_time.min--;
            }

            // lcd_1621_refresh();
        }
    }
    break;
    // ===================================================================
    case KEY4_AD_LONG:
    {
    }
    break;
    // ===================================================================
    case KEY5_AD_CLICK:
    {
        if (sequencer_status == SEQUENCER_STATUS_NONE)
        {
            // 普通模式，翻转继电器的状态

            if (DEVICE_ON == sequencers.on_ff && 0 == is_sequencer_in_delay())
            {
                // 如果没有开机，或者设备正在开关机的延时中，不处理该按键事件
                return;
            }
            sequencer_relay_status_toggle(RELAY_INDEX_5);
        }
        else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
        {
            // 如果正在设置系统时间



            // 递增当前调节的时间 
            time_unit_t cur_setting_time_unit = 0;
            lcd_setting_sys_time_unit_get(&cur_setting_time_unit);
            lcd_setting_sys_time_animation_fix();

            if (cur_setting_time_unit == TIME_UNIT_YEAR)
            {
                cur_setting_sys_time.year++;
            }
            else if (cur_setting_time_unit == TIME_UNIT_MONTH)
            {
                cur_setting_sys_time.month++;
            }
            else if (cur_setting_time_unit == TIME_UNIT_DAY)
            {
                cur_setting_sys_time.day++;
            }
            else if (cur_setting_time_unit == TIME_UNIT_HOUR)
            {
                cur_setting_sys_time.hour++;
            }
            else if (cur_setting_time_unit == TIME_UNIT_MIN)
            {
                cur_setting_sys_time.min++;
            }

            // lcd_1621_refresh();
        }
    }
    break;
    // ===================================================================
    case KEY5_AD_LONG:
    {
        // 长按事件，进入或退出 独立的开关计划
    }
    break;
    // ===================================================================
    case KEY6_AD_CLICK:
    {
        if (sequencer_status == SEQUENCER_STATUS_NONE)
        {
            // 普通模式，翻转继电器的状态
            if (DEVICE_ON == sequencers.on_ff && 0 == is_sequencer_in_delay())
            {
                // 如果没有开机，或者设备正在开关机的延时中，不处理该按键事件
                return;
            }

            sequencer_relay_status_toggle(RELAY_INDEX_6);
        }
        else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
        {
            // 如果正在设置系统时间
            // 将当前设置的时间单位切换为上一个时间单位（顺序：分 -> 时 -> 日 -> 月 -> 年）

            time_unit_t cur_setting_time_unit = 0;
            lcd_setting_sys_time_unit_get(&cur_setting_time_unit); // 获取当前调节的时间单位

            if (cur_setting_time_unit == TIME_UNIT_YEAR)
            {
                // lcd_setting_sys_time_unit_change(TIME_UNIT_MONTH);
                // 不再向上切换
            }
            else if (cur_setting_time_unit == TIME_UNIT_MONTH)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_YEAR);
            }
            else if (cur_setting_time_unit == TIME_UNIT_DAY)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_MONTH);
            }
            else if (cur_setting_time_unit == TIME_UNIT_HOUR)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_DAY);
            }
            else if (cur_setting_time_unit == TIME_UNIT_MIN)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_HOUR);
            }

            // lcd_1621_refresh();
        }

    }
    break;
    // =====================================================================
    case KEY6_AD_LONG:
    {

    }
    break;
    // ===================================================================
    case KEY7_AD_CLICK:
    {
        if (sequencer_status == SEQUENCER_STATUS_NONE)
        {
            // 普通模式，翻转继电器的状态
            if (DEVICE_ON == sequencers.on_ff && 0 == is_sequencer_in_delay())
            {
                // 如果没有开机，或者设备正在开关机的延时中，不处理该按键事件
                return;
            }

            sequencer_relay_status_toggle(RELAY_INDEX_7);
        }
        else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
        {
            // 如果正在设置系统时间
            // 将当前设置的时间单位切换为下一个时间单位（顺序：年 -> 月 -> 日 -> 时 -> 分）
            time_unit_t cur_setting_time_unit = 0;
            lcd_setting_sys_time_unit_get(&cur_setting_time_unit); // 获取当前调节的时间单位

            if (cur_setting_time_unit == TIME_UNIT_YEAR)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_MONTH);
            }
            else if (cur_setting_time_unit == TIME_UNIT_MONTH)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_DAY);
            }
            else if (cur_setting_time_unit == TIME_UNIT_DAY)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_HOUR);
            }
            else if (cur_setting_time_unit == TIME_UNIT_HOUR)
            {
                lcd_setting_sys_time_unit_change(TIME_UNIT_MIN);
            }
            else if (cur_setting_time_unit == TIME_UNIT_MIN)
            {
                // lcd_setting_sys_time_unit_change(TIME_UNIT_HOUR);

                // 不再向下切换
            }

            // lcd_1621_refresh(); 
        }
    }
    break;
    // ===================================================================
    case KEY7_AD_LONG:
    {

    }
    break;
    // ===================================================================

    default:
    {
        return;
    }
    break;

    }// switch (keyevent)


    os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
}