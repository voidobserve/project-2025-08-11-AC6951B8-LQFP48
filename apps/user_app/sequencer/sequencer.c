#include "sequencer.h"
#include "../../../apps/user_app/user_config.h"

#include "../../user_app/flash_handle/flash_handle.h" // flash读写接口

volatile u8 flag_is_lcd_screen_on = 0; // 标志位，lcd屏幕状态，0--未点亮，1--点亮

volatile ON_OFF_FLAG temp_on_off[16];  //继电器的开关

volatile SEQUENCER_T  sequencers;

/**
 * @brief 更新 记录的时序器 指定继电器 的状态（不在开关机期间调用）
 *
 * @param relay_index 指定的继电器索引
 * @param relay_status 继电器状态
 *          RELAY_STATUS_ACTIVE   激活
 *          RELAY_STATUS_DEACTIVE 不激活
 * @return * void
 */
void sequencer_relay_status_update(relay_index_t relay_index, relay_status_t relay_status)
{
    sequencers.relay[relay_index].cur_status_on_off = relay_status;
    sequencers.relay[relay_index].last_status_on_off = relay_status;
}

/**
 * @brief 设置 时序器对应的继电器的状态，会操作继电器，同时更新lcd显示
 *      设置完成后，会更新对应的继电器状态记录
 *
 *      注意： 不能在开关机期间调用
 */
void sequencer_relay_status_setting(relay_index_t relay_index, relay_status_t relay_status)
{
    relay_status_setting(relay_index, relay_status);

    if (RELAY_STATUS_ACTIVE == relay_status)
    {
        lcd_relay_icon_show(relay_index);
    }
    else
    {
        lcd_relay_icon_unshow(relay_index);
    }

    sequencer_relay_status_update(relay_index, relay_status);
}

/**
 * @brief 反转 时序器对应的继电器的状态，会操作继电器，同时更新lcd显示
 *      设置完成后，会更新对应的继电器状态记录
 *
 *      注意： 不能在开关机期间调用
 */
void sequencer_relay_status_toggle(relay_index_t relay_index)
{
    relay_status_toggle(relay_index);

    if (RELAY_STATUS_ACTIVE == relay_status_get(relay_index))
    {
        lcd_relay_icon_show(relay_index);
        sequencer_relay_status_update(relay_index, RELAY_STATUS_ACTIVE);
    }
    else
    {
        lcd_relay_icon_unshow(relay_index);
        sequencer_relay_status_update(relay_index, RELAY_STATUS_DEACTIVE);
    }
}

void sequencer_relay_status_setting_dly(relay_index_t relay_index, relay_status_t relay_status , u8 delay_time)
{
    os_time_dly(delay_time);
    sequencer_relay_status_setting(relay_index, relay_status);
}


void user_msg_handle_task(void* p)
{
    int msg[32] = { 0 };


    while (1)
    {
        // os_sem_pend(msg, 0); // 一直阻塞等待信号量
        int ret = os_taskq_pend("msg_task", msg, 1);
        // printf("recv msg\n");
        // printf("ret %d\n", ret);
        if (OS_TASKQ != ret)
        {
            continue;
        }

        if (msg[0] != Q_USER) // 如果不是用户消息
        {
            continue;
        }

        // for (u8 i =0; i  < ARRAY_SIZE(msg); i++)
        // {
        //     printf("msg [%u]: %d\n", (u16)i, msg[i]);
        // }


        switch (msg[1])
        {
        case MSG_USER_SAVE_INFO: // 如果要保存信息
        {
            // printf("recv msg MSG_USER_SAVE_INFO \n");
            save_user_data_enable(); // 使能保存数据的倒计时，使能保存数据的操作
        }
        break;

        }
    }



    // if (MSG_SEQUENCER_SAVE_INFO)

}