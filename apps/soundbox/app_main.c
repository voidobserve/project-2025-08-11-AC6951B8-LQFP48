#include "system/includes.h"
#include "app_config.h"
#include "asm/pwm_led.h"
#include "tone_player.h"
#include "ui_manage.h"
#include "app_main.h"
#include "app_task.h"
#include "asm/charge.h"
#include "app_power_manage.h"
#include "app_charge.h"
#include "user_cfg.h"
#include "power_on.h"
#include "bt.h"
#include "soundcard/peripheral.h"
#include "audio.h"
#include "vm.h"


#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"



APP_VAR app_var;


void app_entry_idle()
{
    app_task_switch_to(APP_IDLE_TASK);
}


void app_entry_bt()
{
    app_task_switch_to(APP_BT_TASK);
}


void app_entry_sleep()
{
    app_task_switch_to(APP_SLEEP_TASK);
}


void app_task_loop()
{
    while (1) {
        switch (app_curr_task) {
        case APP_POWERON_TASK:
            log_info("APP_POWERON_TASK \n");
            app_poweron_task();
            break;
        case APP_POWEROFF_TASK:
            log_info("APP_POWEROFF_TASK \n");
            app_poweroff_task();
            break;
        case APP_BT_TASK:
            log_info("APP_BT_TASK \n");
            app_bt_task();
            break;
        case APP_MUSIC_TASK:
            log_info("APP_MUSIC_TASK \n");
            app_music_task();
            break;
        case APP_FM_TASK:
            log_info("APP_FM_TASK \n");
            app_fm_task();
            break;
        case APP_RECORD_TASK:
            log_info("APP_RECORD_TASK \n");
            app_record_task();
            break;
        case APP_LINEIN_TASK:
            log_info("APP_LINEIN_TASK \n");
            app_linein_task();
            break;
        case APP_RTC_TASK:
            log_info("APP_RTC_TASK \n");
            app_rtc_task();
            break;
        case APP_PC_TASK:
            log_info("APP_PC_TASK \n");
            app_pc_task();
            break;
        case APP_SPDIF_TASK:
            log_info("APP_SPDIF_TASK \n");
            app_spdif_task();
            break;
        case APP_IDLE_TASK:
            log_info("APP_IDLE_TASK \n");
            app_idle_task();
            break;
        case APP_SLEEP_TASK:
            log_info("APP_SLEEP_TASK \n");
            app_sleep_task();
            break;
        case APP_SMARTBOX_ACTION_TASK:
            log_info("APP_SMARTBOX_ACTION_TASK \n");
            app_smartbox_task();
            break;
        }
        app_task_clear_key_msg();//清理按键消息
        //检查整理VM
        vm_check_all(0);
    }
}


//  -------------------------  定时器  ------------------------------------------




struct timer_hdl {
    int index;
    int prd;
};

static struct timer_hdl hdl;

#define __this  (&hdl)

static const u32 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};

#define APP_TIMER_CLK           clk_get("timer")
#define MAX_TIME_CNT            0x7fff
#define MIN_TIME_CNT            0x100


#define TIMER_CON               JL_TIMER2->CON
#define TIMER_CNT               JL_TIMER2->CNT
#define TIMER_PRD               JL_TIMER2->PRD
#define TIMER_VETOR             IRQ_TIME2_IDX

#define TIMER_UNIT_MS           1 //1ms起一次中断
#define MAX_TIMER_PERIOD_MS     (1000/TIMER_UNIT_MS)

/*-----------------------------------------------------------*/

static volatile u32 delay_cnt = 0;

extern void sputchar(char c);


void (*timer_led_scan)(void *param);

void app_timer_led_scan(void (*led_scan)(void *))
{
    timer_led_scan = led_scan;
}




extern unsigned char dis_data[32];

/////下面函数调用的使用函数都必须放在ram
___interrupt
AT_VOLATILE_RAM_CODE
static void timer2_isr()
{
    static u32 cnt1 = 0;
   
  
   
    TIMER_CON |= BIT(14);


    ir_detect_isr(); //红外遥控解码

    ++cnt1;
  //加入其他内容，会影响红外的解码

}

int timer2_init()
{
    u32 prd_cnt;
    u8 index;

    printf("------------%s :%d", __func__, __LINE__);

    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = TIMER_UNIT_MS * (APP_TIMER_CLK / 1000 / 10) / timer_div[index];  //100us
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            break;
        }
    }
    __this->index   = index;
    __this->prd     = prd_cnt;

    TIMER_CNT = 0;
    TIMER_PRD = prd_cnt; //1ms
    request_irq(TIMER_VETOR, 1, timer2_isr, 0);
    TIMER_CON = (index << 4) | BIT(0) | BIT(3);

    printf("PRD : 0x%x / %d", TIMER_PRD, clk_get("timer"));

    return 0;
}
__initcall(timer2_init);




//  -------------------------  定时器 end  ---------------------------------------









void app_main()
{
    log_info("app_main\n");

    app_var.start_time = timer_get_ms();


    if (get_charge_online_flag()) {

        app_var.poweron_charge = 1;

#if (TCFG_SYS_LVD_EN == 1)
        vbat_check_init();
#endif

#ifndef  PC_POWER_ON_CHARGE
        app_curr_task = APP_IDLE_TASK;
#else
        app_curr_task = APP_POWERON_TASK;
#endif
    } else {
#if SOUNDCARD_ENABLE
        soundcard_peripheral_init();
#endif

#if TCFG_HOST_AUDIO_ENABLE
        void usb_host_audio_init(int (*put_buf)(void *ptr, u32 len), int *(*get_buf)(void *ptr, u32 len));
        usb_host_audio_init(usb_audio_play_put_buf, usb_audio_record_get_buf);
#endif

        /* endless_loop_debug_int(); */
        ui_update_status(STATUS_POWERON);

        app_curr_task = APP_POWERON_TASK;
    }

#if TCFG_CHARGE_BOX_ENABLE
    app_curr_task = APP_IDLE_TASK;
#endif


    // app_entry_bt();

    app_entry_sleep();
    
    app_task_loop();
}



