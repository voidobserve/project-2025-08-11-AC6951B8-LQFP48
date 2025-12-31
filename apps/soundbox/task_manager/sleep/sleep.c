#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_task.h"
#include "tone_player.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "app_main.h"
#include "app_online_cfg.h"
#include "app_power_manage.h"
#include "gSensor/gSensor_manage.h"
#include "ui_manage.h"
#include "vm.h"
#include "app_chargestore.h"
#include "key_event_deal.h"
#include "asm/pwm_led.h"
#include "user_cfg.h"
#include "sleep/sleep.h"
#include "ui/ui_api.h"
#include "clock_cfg.h"
#include "dev_manager.h"
#include "user_api/app_status_api.h"
#include "adkey.h"

#include "../../../../apps/user_app/sequencer/sequencer.h"
#include "user_io_key.h"
#include "user_ad_key.h"

#if TCFG_APP_SLEEP_EN

extern void app_status_handler(enum APP_STATUS status);

//*----------------------------------------------------------------------------*/
/**@brief    sleep 按键消息入口
  @param    无
  @return   1、消息已经处理，不需要发送到common  0、消息发送到common处理
  @note
 */
 /*----------------------------------------------------------------------------*/
static int sleep_key_event_opr(struct sys_event* event)
{
    int ret = false;
    // int err = 0;
    // //使用處理事件/消息的思想，不是鍵值
    int key_event = event->u.key.event;
    int key_value = event->u.key.value;
    u8 key_event_type = event->u.key.type; // 存放按键类型

    // log_i("key_event:%d \n", key_event);

    // printf("%s\n", __func__);

#if 0
    if (key_event_type == KEY_DRIVER_TYPE_IO && // 是io按键事件
        (0 == is_sequencer_in_delay() &&   // 没有在开关机的计时
            (KEW_PROW_IO == key_event || KEY_PROW_IO_LONG == key_event))  // 总开关短按或长按事件触发，并且此时没有在开关机的计时
        )
    {
        extern void iokey_master_on_off(void);
        iokey_master_on_off();
    }
    else if (key_event == KEY_PROW_IO_LONG)
    {

    }
#endif

    if (key_event_type == KEY_DRIVER_TYPE_IO)
    {
        // 需要在按键事件内部判断是否在开关机计时中，设备是否开机
        io_key_event_handle(key_event);
    }

    // 如果是ad按键事件
    if (key_event_type == KEY_DRIVER_TYPE_AD)
    {
#if 0
        if (DEVICE_ON == sequencers.on_ff && 0 == is_sequencer_in_delay())
        { // 单击ad按键  开机状态且计时完成 
            extern void ad_key_event_handle(int keyevent);
            ad_key_event_handle(key_event);
        }
#endif

        // 需要在按键事件内部判断是否在开关机计时中，设备是否开机
        // printf("ad key event == %d\n", key_event);
        ad_key_event_handle(key_event);
    }

    if (key_event_type == KEY_DRIVER_TYPE_IR)
    {
        // 如果是红外按键事件 
        extern void ir_key_event_handle(int key_event);
        ir_key_event_handle(key_event);
    }



    return ret;
}

//*----------------------------------------------------------------------------*/
/**@brief    sleep 模式初始化
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void sleep_task_start(void)
{
    sys_key_event_enable();  //事件通知函数,系统有事件发生时调用此函数

    clock_idle(REC_IDLE_CLOCK);
}


//*----------------------------------------------------------------------------*/
/**@brief    sleep 退出
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/

static void sleep_task_close()
{
}

//*----------------------------------------------------------------------------*/
/**@brief    sleep 模式活跃状态 所有消息入口
   @param    无
   @return   1、当前消息已经处理，不需要发送comomon
             0、当前消息不是linein处理的，发送到common统一处理
   @note
*/
/*----------------------------------------------------------------------------*/
static int sleep_sys_event_handler(struct sys_event* event)
{
    const char* logo = NULL;
    int err = 0;
    switch (event->type)
    {

    case SYS_KEY_EVENT:   //  时序器 处理按键事件

        sleep_key_event_opr(event);
        return 1;

    case SYS_DEVICE_EVENT:
        ///所有设备相关的事件不能返回true， 必须给留给公共处理的地方响应设备上下线
        // printf("event->arg = %d",event->arg);
        switch ((u32)event->arg)
        {
        case DRIVER_EVENT_FROM_SD0:
        case DRIVER_EVENT_FROM_SD1:
        case DRIVER_EVENT_FROM_SD2:
            logo = (char*)event->u.dev.value;
        case DEVICE_EVENT_FROM_OTG:
            if ((u32)event->arg == DEVICE_EVENT_FROM_OTG)
            {
                logo = (char*)"udisk0";
            }
            if (event->u.dev.event == DEVICE_EVENT_IN)
            {
            }
            else if (event->u.dev.event == DEVICE_EVENT_OUT)
            {
            }
            break;//DEVICE_EVENT_FROM_USB_HOST
        }//switch((u32)event->arg)
        break;//SYS_DEVICE_EVENT





    }//switch (event->type)

    return false;
}


//*----------------------------------------------------------------------------*/
/**@brief    sleep 主任务
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void app_sleep_task()
{
    int res;
    int msg[32];
    sleep_task_start();  //sleep模式初始化



    while (1) {
        //while循环一次会阻塞在这里等待msg
        // printf("sleep task circle\n");

        // gpio_direction_output(pwoer_light, 1); //电源指示灯
        // check_relay_start();
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        // printf("msg[0] = %d",msg[0]);
        switch (msg[0])
        {
        case APP_MSG_SYS_EVENT:
            // printf("msg[1] = %d",msg[1]);
            if (sleep_sys_event_handler((struct sys_event*)(&msg[1])) == false)  //AD按键执行sys event
            {
                app_default_event_deal((struct sys_event*)(&msg[1]));    //由common统一处理
                // printf("common handle");
            }
            break;
        default:
            break;
        }

        if (app_task_exitting()) {
            printf("exitting sleep task");
            sleep_task_close();
            return;
        }
    }
}

#else


#endif
