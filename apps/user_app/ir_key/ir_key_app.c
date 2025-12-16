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


#include "../../../apps/soundbox/include/key_event_deal.h" // 按键事件定义
#include "../../../apps/user_app/sequencer/sequencer.h" 





// -------------------------------- 红外遥控  ---------------------------

void ir_key_event_handle(int keyevent)
{
    // printf("%s\n", __func__);

    // u8 next_data[7];
    switch (keyevent)
    {
        // 开机
    case KEY1_IR_CLICK:
    {
        if (sequencers.on_ff == DEVICE_OFF)    // ---------------------- 开机　
        {
            // 这里如果快速短按按键，会重复进入，目前在sequencer_power_on()内部判断是否处于延时，防止重复触发 
            printf("ir key_master_on_off open\n");
            sequencer_power_on();
            return;
        }
        else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机 
        {
            // 这里如果快速短按按键，会重复进入，目前在sequencer_power_off()内部判断是否处于延时，防止重复触发
            printf("ir key_master_on_off off\n");
            sequencer_power_off();
            return;
        }
    }
    break;
    // ================================================================================

    case KEY13_IR_CLICK:   //继电器1  USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw1_led, 0);

            // // 继电器状态变化后，立刻更新继电器状态
            // sequencers.relay[0].cur_status_on_off = temp_on_off[0];
            // sequencers.relay[0].last_status_on_off = temp_on_off[0];

            sequencer_relay_status_toggle(RELAY_INDEX_0); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY14_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw2_led, 1);
            // // 继电器状态变化后，立刻更新继电器状态
            // sequencers.relay[1].cur_status_on_off = temp_on_off[1];
            // sequencers.relay[1].last_status_on_off = temp_on_off[1];

            sequencer_relay_status_toggle(RELAY_INDEX_1); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY15_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw3_led, 2);   fd_relay_state();
            // // 继电器状态变化后，立刻更新继电器状态
            // sequencers.relay[2].cur_status_on_off = temp_on_off[2];
            // sequencers.relay[2].last_status_on_off = temp_on_off[2];

            sequencer_relay_status_toggle(RELAY_INDEX_2); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY16_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw4_led, 3);   fd_relay_state();
            // sequencers.relay[3].cur_status_on_off = temp_on_off[3];
            // sequencers.relay[3].last_status_on_off = temp_on_off[3];

            sequencer_relay_status_toggle(RELAY_INDEX_3); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY17_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw5_led, 4);   fd_relay_state();
            // sequencers.relay[4].cur_status_on_off = temp_on_off[4];
            // sequencers.relay[4].last_status_on_off = temp_on_off[4];

            sequencer_relay_status_toggle(RELAY_INDEX_4); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY18_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw6_led, 5);   fd_relay_state();
            // sequencers.relay[5].cur_status_on_off = temp_on_off[5];
            // sequencers.relay[5].last_status_on_off = temp_on_off[5];

            sequencer_relay_status_toggle(RELAY_INDEX_5); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY19_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw7_led, 6);   fd_relay_state();
            // sequencers.relay[6].cur_status_on_off = temp_on_off[6];
            // sequencers.relay[6].last_status_on_off = temp_on_off[6];

            sequencer_relay_status_toggle(RELAY_INDEX_6); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    // ================================================================================
    case KEY20_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            // adkey_control(sw8_led, 7);   fd_relay_state();
            // sequencers.relay[7].cur_status_on_off = temp_on_off[7];
            // sequencers.relay[7].last_status_on_off = temp_on_off[7];

            sequencer_relay_status_toggle(RELAY_INDEX_7); // 继电器 状态反转
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    } // switch (keyevent)





}





