#include "user_io_key.h"
#include "key_event_deal.h"

#include "user_config.h"

#include "user_sys_time.h"


/**
 * @brief 处理传入的io按键事件
 *
 * @param key_event
 */
void io_key_event_handle(int key_event)
{
    switch (key_event)
    {
    case KEW_PROW_IO: // 按键短按
    {
        if (is_sequencer_in_delay())
        {
            // 如果正在开关机的延时中，不处理该事件
            return;
        }

        if (sequencer_status == SEQUENCER_STATUS_NONE)
        {
            extern void iokey_master_on_off(void);
            iokey_master_on_off();
            return;
        }

        // 如果当前时序器不是空闲状态，会进入到这里
    }
    break;
    // ===================================================================
    case KEY_PROW_IO_LONG: // 按键长按
    {
        if (is_sequencer_in_delay())
        {
            // 如果正在开关机的延时中，不处理该事件 
            return;
        }

        // printf("io key event long\n");

        if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
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
        else if (sequencer_status == SEQUENCER_STATUS_NONE)
            // else
        {
            // 进入设置系统时间的模式
            user_sys_time_t user_sys_time = { 0 };
            user_sys_time_get(&user_sys_time);

            // 由于当前通过按键设置的系统时间，没有秒这一单位，所以将秒的值清零
            user_sys_time.sec = 0;
            user_setting_time_set(&user_sys_time);
            lcd_setting_sys_time_unit_change(TIME_UNIT_YEAR);

            sequencer_status = SEQUENCER_STATUS_SETTING_SYS_TIME;
            printf("setting sys time begin\n");
        }
    }
    break;
    // ===================================================================
    default:
    {
        return;
    }
    break;
    }



}