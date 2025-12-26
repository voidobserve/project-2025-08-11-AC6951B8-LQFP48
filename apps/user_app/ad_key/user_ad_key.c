#include "user_ad_key.h"
#include "key_event_deal.h"
#include "relay_handle.h"
#include "sequencer.h"
#include "lcd1621.h"
#include "user_sys_time.h"

void ad_key_event_handle_in_normal_mode(int key_event)
{
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
        // 长按事件，进入或退出 独立的开关计划
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_POWER_ON_SCHEDULE;
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
        // 长按事件，进入或退出 独立的开关计划
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

    }
    break;
    // ===================================================================

    default:
    {
        return;
    }
    break;

    }// switch (keyevent)
}

void ad_key_event_handle_in_setting_sys_time_mode(int key_event)
{
    switch (key_event)
    {
    case KEY4_AD_CLICK:
    case KEY_4_AD_HOLD:
    {
        // 如果正在设置系统时间
        // 递减当前调节的时间   
        user_setting_time_param_sub();
        lcd_setting_sys_time_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ===================================================================
    case KEY5_AD_CLICK:
    case KEY_5_AD_HOLD:
    {
        user_setting_time_param_add();
        lcd_setting_sys_time_animation_fix();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间 
    }
    break;
    // ===================================================================
    case KEY6_AD_CLICK:
    case KEY6_AD_LONG:
        // case KEY_6_AD_HOLD:
    {
        // 如果正在设置系统时间
        // 将当前设置的时间单位切换为上一个时间单位（顺序：分 -> 时 -> 日 -> 月 -> 年） 
        lcd_setting_sys_time_unit_switch_to_prev();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ===================================================================
    case KEY7_AD_CLICK:
    case KEY7_AD_LONG:
        // case KEY_7_AD_HOLD: 
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
    else
    {
        // 测试时使用，这里存放还未封装好的程序

        switch(keyevent)
        {

            default:
            {
                
            }
            break;
        }
    }

    os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
}