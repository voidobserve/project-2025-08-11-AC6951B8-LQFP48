#include "ir_key_app.h"
#include "key_driver.h"
#include "irkey.h"
#include "gpio.h"
#include "asm/irflt.h"
#include "app_config.h"
#include "system/event.h"
#include "asm/uart_dev.h"
#include "includes.h"
#include "asm/cpu.h"
#include "asm/irq.h"
#include "asm/clock.h"
#include "system/init.h"
#include "debug.h"


#include "key_event_deal.h" // 按键事件定义
#include "sequencer/sequencer.h" 
#include "sequencer/sequencer_device_on_off.h"
#include "user_sys_time.h"
#include "user_lcd_handle.h"
#include "lcd_setting_relay_schedule.h"

/**
 * @brief 在普通模式下，处理对应的按键事件
 *
 * @param key_event
 */
void ir_key_event_handle_in_normal_mode(int key_event)
{
    if (is_sequencer_in_delay())
    {
        // 如果正在开关机延时，不处理对应按键事件
        return;
    }

    if (sequencers.on_ff == DEVICE_OFF &&
        (key_event == KEY_IR_R5C1_CLICK ||
            key_event == KEY_IR_R5C2_CLICK ||
            key_event == KEY_IR_R5C3_CLICK ||
            key_event == KEY_IR_R6C1_CLICK ||
            key_event == KEY_IR_R6C2_CLICK ||
            key_event == KEY_IR_R6C3_CLICK ||
            key_event == KEY_IR_R7C1_CLICK ||
            key_event == KEY_IR_R7C2_CLICK))
    {
        // 设备未开机，不执行继电器相关的激活/停用操作
        return;
    }

    switch (key_event)
    {
    case KEY_IR_R1C1_CLICK:
    {
        if (sequencers.on_ff == DEVICE_OFF)    // ---------------------- 开机　
        {
            printf("ir key_master_on_off open\n");
            sequencer_power_on();
        }
        else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机 
        {
            printf("ir key_master_on_off off\n");
            sequencer_power_off();
        }
    }
    break;
    // ===================================================================
    case KEY_IR_R1C3_CLICK:
    {
        // 普通模式 -> 进入设置系统时间的模式 

        user_sys_time_t user_sys_time = { 0 };
        user_sys_time_get(&user_sys_time);
        // 由于当前通过按键设置的系统时间，没有秒这一单位，所以将秒的值清零
        user_sys_time.sec = 0; 
        user_setting_time_set(&user_sys_time);
        lcd_setting_sys_time_unit_change(TIME_UNIT_YEAR);
        sequencer_status = SEQUENCER_STATUS_SETTING_SYS_TIME; // 让 lcdseg_handle() 扫描到，切换显示

        printf("setting sys time begin\n");
    }
    break;
    // ===================================================================
    case KEY_IR_R5C1_CLICK:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_0); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
    }
    break;
    // ===================================================================
    case KEY_IR_R5C2_CLICK: // 
    {
        sequencer_relay_status_toggle(RELAY_INDEX_1); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
    }
    break;
    // ===================================================================
    case KEY_IR_R5C3_CLICK: //  
    {
        sequencer_relay_status_toggle(RELAY_INDEX_2); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程 
    }
    break;
    // ================================================================================
    case KEY_IR_R6C1_CLICK: // 
    {
        sequencer_relay_status_toggle(RELAY_INDEX_3); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程 
    }
    break;
    // ===================================================================
    case KEY_IR_R6C2_CLICK: //  
    {
        sequencer_relay_status_toggle(RELAY_INDEX_4); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程 
    }
    break;
    // ================================================================================
    case KEY_IR_R6C3_CLICK: // 
    {
        sequencer_relay_status_toggle(RELAY_INDEX_5); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程 
    }
    break;
    // ================================================================================
    case KEY_IR_R7C1_CLICK: //  
    {
        sequencer_relay_status_toggle(RELAY_INDEX_6); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程 
    }
    break;
    // ================================================================================
    case KEY_IR_R7C2_CLICK: //  
    {
        sequencer_relay_status_toggle(RELAY_INDEX_7); // 继电器 状态反转
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程 
    }
    break;
    // ================================================================================
    default:
    {

    }
    break;
    }


}

/**
 * @brief 在设置系统时间的模式下，处理对应的按键事件
 *
 * @param key_event
 */
void ir_key_event_handle_in_setting_sys_time_mode(int key_event)
{
    switch (key_event)
    {
    case KEY_IR_R1C3_CLICK:
    {
        // 如果正在设置系统时间，则退出设置
        

        // 保存设置的时间
        user_sys_time_t user_sys_time = { 0 };
        user_setting_time_get(&user_sys_time);
        user_sys_time_set(&user_sys_time);
        printf("setting sys time exit\n");
        lcd_refresh_time_reset();
        sequencer_status = SEQUENCER_STATUS_NONE;
        // 退出设置之后，保存相关的用户数据
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ================================================================================
    case KEY_IR_R3C1_CLICK:
    {
        // 递减当前调节的时间
        user_setting_time_param_sub();
        lcd_setting_sys_time_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ================================================================================
    case KEY_IR_R3C2_CLICK:
    {
        // 递增当前调节的时间
        user_setting_time_param_add();
        lcd_setting_sys_time_animation_fix();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间 
    }
    break;
    // ================================================================================
    case KEY_IR_R4C1_CLICK:
    {
        // 将当前设置的时间单位切换为上一个时间单位 （顺序：分 -> 时 -> 日 -> 月 -> 年） 
        lcd_setting_sys_time_unit_switch_to_prev();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ================================================================================
    case KEY_IR_R4C2_CLICK:
    {
        // 将当前设置的时间单位切换为下一个时间单位（顺序：年 -> 月 -> 日 -> 时 -> 分）  
        lcd_setting_sys_time_unit_switch_to_next();
        lcd_setting_sys_time_timeout_reset(); // 清空超时时间  
    }
    break;
    // ================================================================================
    case KEY_IR_R5C1_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_0 
        lcd_setting_relay_active_constructor(RELAY_INDEX_0);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R5C2_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_1 
        lcd_setting_relay_active_constructor(RELAY_INDEX_1);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R5C3_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_2
        lcd_setting_relay_active_constructor(RELAY_INDEX_2);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R6C1_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_3
        lcd_setting_relay_active_constructor(RELAY_INDEX_3);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R6C2_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_4
        lcd_setting_relay_active_constructor(RELAY_INDEX_4);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R6C3_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_5
        lcd_setting_relay_active_constructor(RELAY_INDEX_5);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    // ================================================================================
    case KEY_IR_R7C1_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_6
        lcd_setting_relay_active_constructor(RELAY_INDEX_6);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R7C2_CLICK:
    {
        // 进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_7
        lcd_setting_relay_active_constructor(RELAY_INDEX_7);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    default:
    {

    }
    break;
    }
}

/**
 * @brief 在设置继电器的定时激活、定时停用的模式下，处理对应的按键事件
 *
 * @param key_event
 */
void ir_key_event_handle_in_setting_relay_schedule_mode(int key_event)
{
    user_sys_time_t active_time = { 0 }; // 存放定时激活时间
    user_sys_time_t deactive_time = { 0 }; // 存放定时停用时间
    relay_index_t relay_index = RELAY_INDEX_INVALID; // 继电器索引

    switch (key_event)
    {
    case KEY_IR_R1C3_CLICK:
    {
        // 退出设置继电器定时计划的模式：
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        lcd_refresh_time_reset();
        sequencer_status = SEQUENCER_STATUS_NONE; // 退出设置继电器定时计划的模式
        // 保存用户数据
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ================================================================================
    case KEY_IR_R2C3_CLICK:
    {
        // 退出设置 继电器定时计划 的模式，进入 设置系统时间 的模式
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);

        user_sys_time_t user_sys_time = { 0 };
        user_sys_time_get(&user_sys_time);
        // 由于当前通过按键设置的系统时间，没有秒这一单位，所以将秒的值清零
        user_sys_time.sec = 0; 
        user_setting_time_set(&user_sys_time);
        lcd_setting_sys_time_unit_change(TIME_UNIT_YEAR);
        sequencer_status = SEQUENCER_STATUS_SETTING_SYS_TIME; // 让 lcdseg_handle() 扫描到，切换显示
        printf("setting sys time begin\n");
    }
    break;
    // ================================================================================
    case KEY_IR_R3C1_CLICK:
    {
        // 设置定时器的定时计划期间，递减 当前调节的时间
        lcd_setting_relay_time_param_sub();
        lcd_setting_relay_schedule_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_relay_schedule_timeout_reset(); // 清空超时时间
    }
    break;
    // ================================================================================
    case KEY_IR_R3C2_CLICK:
    {
        // 设置定时器的定时计划期间，递增 当前调节的时间
        lcd_setting_relay_time_param_add();
        lcd_setting_relay_schedule_animation_fix(); // 让lcd固定显示一段时间
        lcd_setting_relay_schedule_timeout_reset(); // 清空超时时间
    }
    break;
    // ================================================================================
    case KEY_IR_R4C1_CLICK:
    {
        // 将当前设置的时间单位切换为上一个时间单位
        lcd_setting_relay_time_unit_switch_to_prev();
        lcd_setting_relay_schedule_timeout_reset();
    }
    break;
    // ================================================================================
    case KEY_IR_R4C2_CLICK:
    {
        // 将当前设置 的时间单位切换为下一个时间单位
        lcd_setting_relay_time_unit_switch_to_next();
        lcd_setting_relay_schedule_timeout_reset();
    }
    break;
    // ================================================================================
    case KEY_IR_R5C1_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_0  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置

        lcd_setting_relay_active_constructor(RELAY_INDEX_0);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R5C2_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_1  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置

        lcd_setting_relay_active_constructor(RELAY_INDEX_1);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R5C3_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_2  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置

        lcd_setting_relay_active_constructor(RELAY_INDEX_2);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R6C1_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_3  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        lcd_setting_relay_active_constructor(RELAY_INDEX_3);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R6C2_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_4  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        lcd_setting_relay_active_constructor(RELAY_INDEX_4);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R6C3_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_5  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        lcd_setting_relay_active_constructor(RELAY_INDEX_5);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R7C1_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_6  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        lcd_setting_relay_active_constructor(RELAY_INDEX_6);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    case KEY_IR_R7C2_CLICK:
    {
        // 保存当前继电器的定时计划，再进入继电器对应的定时激活、定时停用设置 RELAY_INDEX_7  
        lcd_setting_relay_schedule_get_index(&relay_index);
        lcd_setting_relay_time_get(&active_time, &deactive_time); // 获取设置好的 定时计划
        weekly_schedule_relay_set(relay_index, active_time, deactive_time); // 根据设置好的定时计划进行设置
        lcd_setting_relay_active_constructor(RELAY_INDEX_7);
        sequencer_status = SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE;// 让 lcdseg_handle() 扫描到，切换显示
    }
    break;
    // ================================================================================
    default:
        break;
    }
}

void ir_key_event_handle(int key_event)
{
    // printf("%s\n", __func__);

    if (sequencer_status == SEQUENCER_STATUS_NONE)
    {
        // 如果在普通模式，转到对应的处理函数
        ir_key_event_handle_in_normal_mode(key_event);
    }
    else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
    {
        // 如果正在 设置系统时间 ，转到对应的处理函数
        ir_key_event_handle_in_setting_sys_time_mode(key_event);
    }
    else if (sequencer_status == SEQUENCER_STATUS_SETTING_RELAY_ACTIVE_SCHEDULE ||
        sequencer_status == SEQUENCER_STATUS_SETTING_RELAY_DEACTIVE_SCHEDULE)
    {
        ir_key_event_handle_in_setting_relay_schedule_mode(key_event);
    }
}





