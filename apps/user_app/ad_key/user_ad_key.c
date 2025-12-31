#include "user_ad_key.h"
#include "key_event_deal.h"
#include "relay_handle.h"
#include "sequencer.h"
#include "sequencer_device_on_off.h"
#include "lcd1621.h"
#include "user_sys_time.h"
#include "lcd_setting_relay_schedule.h"
#include "relay_handle.h" // relay_index_t 

void ad_key_event_handle_in_normal_mode(int key_event)
{
    if (is_sequencer_in_delay())
    {
        // 如果设备正在开关机的延时中，直接返回
        return;
    }

    if (sequencers.on_ff == DEVICE_OFF &&
        (key_event == KEY0_AD_CLICK ||
            key_event == KEY1_AD_CLICK ||
            key_event == KEY2_AD_CLICK ||
            key_event == KEY3_AD_CLICK ||
            key_event == KEY4_AD_CLICK ||
            key_event == KEY5_AD_CLICK ||
            key_event == KEY6_AD_CLICK ||
            key_event == KEY7_AD_CLICK))
    {
        // 如果设备未开机，不执行对应的短按事件
        return;
    }

    switch (key_event)
    {
    case KEY0_AD_CLICK: // 第一路对应的继电器按键 
    {
        sequencer_relay_status_toggle(RELAY_INDEX_0);
    }
    break;
    // ===================================================================
    case KEY0_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_0);
    }
    break;
    // ===================================================================
    case KEY1_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_1);
    }
    break;
    // ===================================================================
    case KEY1_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_1);
    }
    break;
    // ===================================================================
    case KEY2_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_2);
    }
    break;
    // ===================================================================
    case KEY2_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_2);
    }
    break;
    // ===================================================================
    case KEY3_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_3);
    }
    break;
    // =================================================================== 
    case KEY3_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_3);
    }
    break;
    // ===================================================================
    case KEY4_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_4);
    }
    break;
    // ===================================================================
    case KEY4_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_4);
    }
    break;
    // ===================================================================
    case KEY5_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_5);
    }
    break;
    // ===================================================================
    case KEY5_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_5);
    }
    break;
    // ===================================================================
    case KEY6_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_6);
    }
    break;
    // =====================================================================
    case KEY6_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_6);
    }
    break;
    // ===================================================================
    case KEY7_AD_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_7);
    }
    break;
    // ===================================================================
    case KEY7_AD_LONG:
    {
        // 长按事件，进入 独立的定时激活、定时停用计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;
        lcd_setting_relay_active_constructor(RELAY_INDEX_7);
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

void ad_key_event_handle_in_setting_sys_time_mode(int key_event)
{
    switch (key_event)
    {
    case KEY4_AD_CLICK: case KEY_4_AD_HOLD:
    {
        // 如果正在设置系统时间
        // 递减当前调节的时间   
        user_setting_time_param_sub();
        lcd_setting_sys_time_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ===================================================================
    case KEY5_AD_CLICK: case KEY_5_AD_HOLD:
    {
        user_setting_time_param_add();
        lcd_setting_sys_time_animation_fix();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间 
    }
    break;
    // ===================================================================
    case KEY6_AD_CLICK: case KEY6_AD_LONG:
    {
        // 如果正在设置系统时间
        // 将当前设置的时间单位切换为上一个时间单位（顺序：分 -> 时 -> 日 -> 月 -> 年） 
        lcd_setting_sys_time_unit_switch_to_prev();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ===================================================================
    case KEY7_AD_CLICK: case KEY7_AD_LONG:
    {
        // 如果正在设置系统时间
        // 将当前设置的时间单位切换为下一个时间单位（顺序：年 -> 月 -> 日 -> 时 -> 分）  
        lcd_setting_sys_time_unit_switch_to_next();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ===================================================================  
    default:
    {
        return;
    }
    break;

    } // switch (keyevent) 
}

void ad_key_event_handle_in_setting_relay_schedule_mode(int key_event)
{
    user_sys_time_t active_time = { 0 }; // 存放定时激活时间
    user_sys_time_t deactive_time = { 0 }; // 存放定时停用时间
    relay_index_t relay_index = RELAY_INDEX_INVALID; // 继电器索引

    switch (key_event)
    {
    case KEY0_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出

        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_0)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time);
        weekly_schedule_relay_set(RELAY_INDEX_0, active_time, deactive_time);

        // printf("\n========================================\n");
        // printf("relay index 0\n");
        // printf("active time == %u : %u\n", (u16)active_time.hour, (u16)active_time.min);
        // printf("deactive time == %u : %u\n", (u16)deactive_time.hour, (u16)deactive_time.min);
        // printf("\n========================================\n");
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    break;
    // ===================================================================
    case KEY1_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出

        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_1)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_1, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    break;
    // ===================================================================
    case KEY2_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出

        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_2)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_2, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    break;
    // ===================================================================
    case KEY3_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出

        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_3)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_3, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    break;
    // =================================================================== 
    case KEY4_AD_CLICK: case KEY_4_AD_HOLD:
    {
        // 如果正在设置对应继电器的定时激活、定时停用计划
        // 递减 当前调节的时间

        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index == RELAY_INDEX_4 && key_event == KEY_4_AD_HOLD)
        {
            // 如果此时正在设置继电器4的定时计划，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_param_sub();
        lcd_setting_relay_schedule_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_relay_schedule_timeout_reset(); // 清空超时时间
    }
    break;
    // ===================================================================
    case KEY4_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出
        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_4)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_4, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    break;
    // =================================================================== 
    case KEY5_AD_CLICK: case KEY_5_AD_HOLD:
    {
        // 如果正在设置对应继电器的定时激活、定时停用计划
        // 递增 当前调节的时间

        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index == RELAY_INDEX_5 && key_event == KEY_5_AD_HOLD)
        {
            // 如果此时正在设置继电器4的定时计划，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_param_add();
        lcd_setting_relay_schedule_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_relay_schedule_timeout_reset(); // 清空超时时间
    }
    break;
    // ===================================================================
    case KEY5_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出
        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_5)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_5, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    break;
    // ===================================================================
    case KEY6_AD_CLICK:
    {
        // 如果正在设置对应继电器的定时激活、定时停用计划
        // 将当前设置的时间单位切换为上一个时间单位
        lcd_setting_relay_time_unit_switch_to_prev();
        lcd_setting_relay_schedule_timeout_reset();
    }
    break;
    // ===================================================================
    case KEY6_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出
        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_6)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_6, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    // ===================================================================
    case KEY7_AD_CLICK:
    {
        // 如果正在设置对应继电器的定时激活、定时停用计划
        // 将当前设置的时间单位切换为下一个时间单位
        lcd_setting_relay_time_unit_switch_to_next();
        lcd_setting_relay_schedule_timeout_reset();
    }
    break;
    // ===================================================================  
    case KEY7_AD_LONG:
    {
        // 长按事件，如果此时设置调节继电器的 定时计划，保存并退出
        lcd_setting_relay_schedule_get_index(&relay_index);
        if (relay_index != RELAY_INDEX_7)
        {
            // 如果继电器索引值不一致，不处理该事件，直接返回
            return;
        }

        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(RELAY_INDEX_7, active_time, deactive_time); // 根据设置好的定时计划进行设置
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
    }
    // ===================================================================  
    default:
    {
        return;
    }
    break;
    }

    os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
}

/**
 * @brief AD按键控制16路继电器
 *
 * @param keyevent    AD按键消息
 */
void ad_key_event_handle(int keyevent)
{
    // printf("ad key event == %d\n", keyevent);
    if (sequencer_status == SEQUENCER_STATUS_NONE)
    {
        // 如果在普通模式，转到对应的处理函数
        ad_key_event_handle_in_normal_mode(keyevent);
    }
    else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
    {
        // 如果正在设置系统时间，转到对应的处理函数
        ad_key_event_handle_in_setting_sys_time_mode(keyevent);
    }
    else if (sequencer_status == SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE ||
        sequencer_status == SEQUENCER_STATUS_SETTING_RELAY_DEACTIVE_SCHEDULE)
    {
        ad_key_event_handle_in_setting_relay_schedule_mode(keyevent);
    }
    else
    {
        // 测试时使用，这里存放还未封装好的程序 
    }

    // os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
}