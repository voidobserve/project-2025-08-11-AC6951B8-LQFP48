
#include "app_config.h"
#include "system/includes.h"
#include "asm/charge.h"
#include "app_power_manage.h"
#include "update.h"
#include "app_main.h"
#include "app_charge.h"
#include "chgbox_ctrl.h"
#include "update_loader_download.h"

#include "../../user_app/user_config.h" // 用户配置头文件
#include "../../user_app/user_main_task.h" // 用户主任务

#include "../../apps/soundbox/include/key_event_deal.h" // 按键消息类型定义
  




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
    }
    else {
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
        }
        else {
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
// void power_light_gpio_init(void)
// {
//     gpio_set_pull_down(pwoer_light, 0);
//     gpio_set_pull_up(pwoer_light, 0);
//     gpio_direction_output(pwoer_light, 0);


// }


//耀祥时序器 MP3的三个灯
void mp3key_light_gpio_init(void)
{
    //上
    gpio_set_pull_down(mp3_light_shang, 0);
    gpio_set_pull_up(mp3_light_shang, 0);
    gpio_direction_output(mp3_light_shang, 0);
    //中
    gpio_set_pull_down(mp3_light_zhong, 0);
    gpio_set_pull_up(mp3_light_zhong, 0);
    gpio_direction_output(mp3_light_zhong, 0);
    //下
    gpio_set_pull_down(mp3_light_xia, 0);
    gpio_set_pull_up(mp3_light_xia, 0);
    gpio_direction_output(mp3_light_xia, 0);

    gpio_direction_output(mp3_light_shang, 0);
    gpio_direction_output(mp3_light_zhong, 0);
    gpio_direction_output(mp3_light_xia, 0);
}

#if 1 // 刚上电，如果要马上开机，使用这段：
volatile u16 lcd_first_pwr_on_timer_id = 0;
// volatile u8 lcd_first_pwr_on_time_cnt = 0;
void lcd_first_pwr_on_isr(void)
{
    // lcd_first_pwr_on_time_cnt++;
    // if (lcd_first_pwr_on_time_cnt >= 2) // 等一段时间后，再点亮屏幕
    // {
    //     lcd_first_pwr_on_time_cnt = 0;
    sys_hi_timer_del(lcd_first_pwr_on_timer_id);
    lcd_open_frame();

    printf("sequencers.on_ff = %u\n", (u16)sequencers.on_ff);
    if (sequencers.on_ff == DEVICE_ON)
    {
        // 如果断电之前是开机的状态
        // find_max_time(DEVICE_ON);
        // open_timer_test();//开始时序

        // sequence_update_max_power_time_before_first_power_on();

        
        printf("===================\n");
        printf("first power on\n");
        printf("device on\n");
        // sequencer_first_power_on();
        sequencer_power_on();
    }
    else
    {
        // 如果断电之前是关机状态
        // 只点亮屏幕，不打开任何继电器
    }
    // }
}


// 刚上电不能马上开机，否则图标会全亮
void lcd_handle_when_first_power_on()
{
    if (lcd_first_pwr_on_timer_id == 0)                  // 防止重复注册
    {
        // lcd_first_pwr_on_timer_id = sys_hi_timer_add(NULL, lcd_first_pwr_on_isr, 500); // 注册定时器 
        lcd_first_pwr_on_timer_id = sys_hi_timer_add(NULL, lcd_first_pwr_on_isr, 200); // 注册定时器 
    }
}
#endif // 刚上电，如果要马上开机

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
    power_light_init();

    // Uart0_Init(); //耀祥串口0  功率计
    Uart1_Init(); //耀祥串口1  向下一级
    Uart2_Init(); //耀祥串口2  连接PC或者上一级的设备

    ac_detection_init(); // 交流电电压检测 

    // power_light_gpio_init();
    // mp3key_light_gpio_init();
    User_rtc_load_save(1); // 初始化系统时间 
    read_sys_current_time();

    // extern void set_open_machine_flag(void);
    save_user_data_init(); // 读取flash信息，初始化相应变量

    extern void lcd1621_init(void);
    lcd1621_init();

    extern void  lcdseg_handle(void); 
    // extern void relay_timer_handle(void);
    sys_timer_add(NULL, lcdseg_handle, 10);  // LCD屏显示处理 

    sys_hi_timer_add(NULL, ac_detection_update, 2); // 采集交流电检测脚上的ad值
    sys_hi_timer_add(NULL, ac_voltage_update, 1500); // 计算、更新交流电电压值

    lcd_handle_when_first_power_on();

    task_create(user_msg_handle_task, NULL, "msg_task");

    // 创建用户的主任务线程
    task_create(user_main_task, NULL , "user_task");
 
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
        }
        else {
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
        printf("reset_flag:0x%x", power_reset_src);
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

        }
        else { //关机时，充电拔出
            power_set_soft_poweroff();
        }
    }
#endif

#if(TCFG_CHARGE_BOX_ENABLE)
    /* clock_add_set(CHARGE_BOX_CLK); */
    chgbox_init_app();
#endif


    // app_task_put_key_msg(KEW_PROW_IO, 0);  //推送按键消息
}

static void app_task_handler(void* p)
{
    app_init();  //app初始化
    app_main();
}

__attribute__((used)) int* __errno()
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

