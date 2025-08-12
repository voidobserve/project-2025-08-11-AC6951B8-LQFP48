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
#if TCFG_APP_SLEEP_EN

extern void app_status_handler(enum APP_STATUS status);

extern SEQUENCER sequencers;

extern void adkey_master_on_off(void);
extern void adkey_16way_on_off(int keyevent);
//*----------------------------------------------------------------------------*/
/**@brief    sleep 按键消息入口
  @param    无
  @return   1、消息已经处理，不需要发送到common  0、消息发送到common处理
  @note
 */
/*----------------------------------------------------------------------------*/
static int sleep_key_event_opr(struct sys_event *event)
{
    int ret = false;
    int err = 0;
// //使用處理事件/消息的思想，不是鍵值
    int key_event = event->u.key.event;
    int key_value = event->u.key.value;
    log_i("key_event:%d \n", key_event);
 


    if(key_event == KEW_PROW_IO  && sequencers.timeing_flag == 1)
    { 
        sequencers.timeing_flag = 0;
        adkey_master_on_off();

        app_task_switch_to(APP_BT_TASK);
    }
    //单击红外按键 仅开机
    if( key_event == KEY1_IR_CLICK && sequencers.timeing_flag == 1)
    {
        sequencers.timeing_flag = 0;
        irkey_16way_click(KEY1_IR_CLICK);
        app_task_switch_to(APP_BT_TASK);
    }


    if(key_event == APP_CMD)  //上位机读取当前状态
    {
        fd_relay_state();
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
static int sleep_sys_event_handler(struct sys_event *event)
{
    const char *logo = NULL;
    int err = 0;
    switch (event->type)
    {

        case SYS_KEY_EVENT:   //博朗时序器AD按键处理消息
        
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
                    logo = (char *)event->u.dev.value;
                case DEVICE_EVENT_FROM_OTG:
                    if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) 
                    {
                        logo = (char *)"udisk0";
                    }
                    if (event->u.dev.event == DEVICE_EVENT_IN) 
                    {
                    } else if (event->u.dev.event == DEVICE_EVENT_OUT) 
                    {
                    }
                    break;//DEVICE_EVENT_FROM_USB_HOST
            }//switch((u32)event->arg)
            break;//SYS_DEVICE_EVENT
        default:
            break;;
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
        printf("sleep");
    
        gpio_direction_output(pwoer_light,1); //电源指示灯
        // check_relay_start();
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        // printf("msg[0] = %d",msg[0]);
        switch (msg[0])
        {
            case APP_MSG_SYS_EVENT:
                // printf("msg[1] = %d",msg[1]);
                if (sleep_sys_event_handler((struct sys_event *)(&msg[1])) == false)  //AD按键执行sys event
                {
                    app_default_event_deal((struct sys_event *)(&msg[1]));    //由common统一处理
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


void app_sleep_task()
{

}

#endif
