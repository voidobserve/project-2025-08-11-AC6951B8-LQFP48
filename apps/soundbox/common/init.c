
#include "app_config.h"
#include "system/includes.h"
#include "asm/charge.h"
#include "app_power_manage.h"
#include "update.h"
#include "app_main.h"
#include "app_charge.h"
#include "chgbox_ctrl.h"
#include "update_loader_download.h"

#include "../../user_app/ac_detection/ac_detection.h"


extern void setup_arch();
extern int audio_dec_init();
extern int audio_enc_init();

static void do_initcall()
{
    __do_initcall(initcall);
}

static void do_early_initcall()
{
    __do_initcall(early_initcall);
}

static void do_late_initcall()
{
    __do_initcall(late_initcall);
}

static void do_platform_initcall()
{
    __do_initcall(platform_initcall);
}

static void do_module_initcall()
{
    __do_initcall(module_initcall);
}

void __attribute__((weak)) board_init()
{

}
void __attribute__((weak)) board_early_init()
{

}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    //1:Endless Sleep
    //0:100 ms wakeup
    if (get_charge_full_flag()) {
        log_i("Endless Sleep");
        power_set_soft_poweroff();
        return 1;
    } else {
        log_i("100 ms wakeup");
        return 0;
    }

}

static void check_power_on_key(void)
{
    u32 delay_10ms_cnt = 0;
    u32 delay_10msp_cnt = 0;

    while (1) {
        clr_wdt();
        os_time_dly(2);

        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            putchar('+');
            delay_10msp_cnt = 0;
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 70) {
                return;
            }
        } else {
            putchar('-');
            delay_10ms_cnt = 0;

            delay_10msp_cnt++;
            if (delay_10msp_cnt > 20) {
                puts("enter softpoweroff\n");
                power_set_soft_poweroff();
            }
        }
    }
}


#include "adkey.h"

//耀祥时序器，电源指示灯，通电就亮
void power_light_gpio_init(void)
{
    gpio_set_pull_down(pwoer_light,0);
    gpio_set_pull_up(pwoer_light,0);
    gpio_direction_output(pwoer_light,0);

    
}


//耀祥时序器 MP3的三个灯
void mp3key_light_gpio_init(void)
{
    //上
    gpio_set_pull_down(mp3_light_shang,0);
    gpio_set_pull_up(mp3_light_shang,0);
    gpio_direction_output(mp3_light_shang,0);
    //中
    gpio_set_pull_down(mp3_light_zhong,0);
    gpio_set_pull_up(mp3_light_zhong,0);
    gpio_direction_output(mp3_light_zhong,0);
    //下
    gpio_set_pull_down(mp3_light_xia,0);
    gpio_set_pull_up(mp3_light_xia,0);
    gpio_direction_output(mp3_light_xia,0);

    gpio_direction_output(mp3_light_shang,0);
    gpio_direction_output(mp3_light_zhong,0);
    gpio_direction_output(mp3_light_xia,0);
}





static void app_init()
{
    printf("app_init\n");
    int update;

    do_early_initcall();
    do_platform_initcall();

    board_init();

    do_initcall();

    do_module_initcall();
    do_late_initcall();

    audio_enc_init();
    audio_dec_init();

  
// ---------- 耀祥时序器  ---------


    // Uart0_Init(); //耀祥串口0  功率计
    Uart1_Init(); //耀祥串口1  向下一级
    Uart2_Init(); //耀祥串口2  连接PC

    ac_detection_init();

    power_light_gpio_init();
    mp3key_light_gpio_init();
    User_rtc_load_save(1); // 初始化系统时间 
    read_sys_current_time();

    extern void set_open_machine_flag(void);
    read_flash_sequencers_status_init();
    extern void lcd1621_init(void);
    lcd1621_init();
    set_open_machine_flag();   // 上电初始化
   
    extern void  lcdseg_handle(void);
    extern void  printf_current_time(void);
    extern void relay_timer_handle(void);
    sys_timer_add(NULL, lcdseg_handle, 10);  //
    sys_timer_add(NULL, relay_timer_handle, 1000); 
    sys_hi_timer_add(NULL, ac_detection_update, 2);
    sys_hi_timer_add(NULL, ac_voltage_update, 500);

// ---------- 耀祥时序器 END --------


    if (!UPDATE_SUPPORT_DEV_IS_NULL()) {
        update = update_result_deal();
    }

    app_var.play_poweron_tone = 1;

    if (!get_charge_online_flag()) {
        check_power_on_voltage();

#if TCFG_POWER_ON_NEED_KEY
        /*充电拔出,CPU软件复位, 不检测按键，直接开机*/
#if TCFG_CHARGE_OFF_POWERON_NE
        if ((!update && cpu_reset_by_soft()) || is_ldo5v_wakeup()) {
#else
        if (!update && cpu_reset_by_soft()) {
#endif
            app_var.play_poweron_tone = 0;
        } else {
            check_power_on_key();
        }
#endif
    }

#if (TCFG_MC_BIAS_AUTO_ADJUST == MC_BIAS_ADJUST_POWER_ON)
    extern u8 power_reset_src;
    u8 por_flag = 0;
    u8 cur_por_flag = 0;
    /*
     *1.update
     *2.power_on_reset(BIT0:上电复位)
     *3.pin reset(BIT4:长按复位)
     */
    if (update || (power_reset_src & BIT(0)) || (power_reset_src & BIT(4))) {
        // log_info("reset_flag:0x%x",power_reset_src);
        printf("reset_flag:0x%x",power_reset_src);
        cur_por_flag = 0xA5;
    }
    int ret = syscfg_read(CFG_POR_FLAG, &por_flag, 1);
    if ((cur_por_flag == 0xA5) && (por_flag != cur_por_flag)) {
        // log_info("update POR flag");
        printf("update POR flag");
        ret = syscfg_write(CFG_POR_FLAG, &cur_por_flag, 1);
    }
#endif

#if (TCFG_CHARGE_ENABLE && TCFG_CHARGE_POWERON_ENABLE)
    if (is_ldo5v_wakeup()) { //LDO5V唤醒
        extern u8 get_charge_online_flag(void);
        if (get_charge_online_flag()) { //关机时，充电插入

        } else { //关机时，充电拔出
            power_set_soft_poweroff();
        }
    }
#endif

#if(TCFG_CHARGE_BOX_ENABLE)
    /* clock_add_set(CHARGE_BOX_CLK); */
    chgbox_init_app();
#endif
}

static void app_task_handler(void *p)
{
    app_init();  //app初始化
    app_main();
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}


int main()
{


#if(CONFIG_CPU_BR25)

#if (TCFG_DEC2TWS_ENABLE ||RECORDER_MIX_EN || TCFG_DRC_ENABLE || TCFG_USER_BLE_ENABLE || TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE || TCFG_USER_EMITTER_ENABLE)
    clock_set_sfc_max_freq(100 * 1000000, 100 * 1000000);
#else

#if ((TCFG_AEC_ENABLE) && (TCFG_USER_TWS_ENABLE))
    clock_set_sfc_max_freq(80 * 1000000, 80 * 1000000);
#else
    clock_set_sfc_max_freq(64 * 1000000, 64 * 1000000);
#endif

#endif

#endif

    wdt_close();

    os_init();

    setup_arch();

    board_early_init();

    task_create(app_task_handler, NULL, "app_core");    //创建app任务

    os_start();

    local_irq_enable();

    while (1) {
        asm("idle");
    }

    return 0;
}

