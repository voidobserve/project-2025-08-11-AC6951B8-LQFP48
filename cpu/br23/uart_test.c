#include "system/includes.h"
#include "system/event.h"
#include "includes.h"
#include "app_task.h"
#include "key_event_deal.h"

// #include "../../apps/user_app/user_config.h"
#include "user_config.h"

// #include "../../apps/user_app/lcd/lcd1621.h"
// #include "../../apps/user_app/sequencer/sequencer.h" // 时序器相关变量类型和变量定义
// #include "../../apps/user_app/sequencer/sequencer_device_on_off.h" // 时序器设备开关控制
// #include "../../apps/user_app/flash_handle/flash_handle.h" // flash读写接口




// -------------------------------------- 时序器功能 ---------------------------------


#include "adkey.h"
// #include "lcd1621.h"



extern u8 display_data[16];   //lcd数据

// 继电器对应的按键灯位置（索引）
const u8 relay_table[RELAYS_MAX] = {
    //按键灯（继电器

    [0] = sw1_led,
    [1] = sw2_led,
    [2] = sw3_led,
    [3] = sw4_led,
    [4] = sw5_led,
    [5] = sw6_led,
    [6] = sw7_led,
    [7] = sw8_led,


}; // 继电器对应的按键灯位置（索引）






/**
 * @brief 本地设备串口信息清零
 *
 * @param RxBuf
 * @param Len
 */
void Controller_MsgDeal(u8* RxBuf, u8 Len)
{
    memset(RxBuf, 0, Len);

}
// /**
//  * @brief 级联设备串口信息清零
//  *
//  * @param RxBuf
//  * @param Len
//  */
// void NextMCU_MsgDeal(u8* RxBuf, u8 Len)
// {
//     memset(RxBuf, 0, Len);

// }

/**
 * @brief 串口0接受数据清零
 *
 * @param RxBuf
 * @param Len
 */
void Uart0_Rx_Deal(u8* RxBuf, u8 Len)
{
    extern void Controller_MsgDeal(u8 * RxBuf, u8 Len);
    Controller_MsgDeal(RxBuf, Len);
}

// /**
//  * @brief 串口1接受数据清零
//  *
//  * @param RxBuf
//  * @param Len
//  */
// void Uart1_Rx_Deal(u8* RxBuf, u8 Len)
// {
//     extern void Controller_MsgDeal(u8 * RxBuf, u8 Len);
//     Controller_MsgDeal(RxBuf, Len);
// }

// #if UART_EN_NUM == 2
// /**
//  * @brief 串口2接受数据清零
//  *
//  * @param RxBuf
//  * @param Len
//  */
// void Uart2_Rx_Deal(u8* RxBuf, u8 Len)
// {
//     extern void NextMCU_MsgDeal(u8 * RxBuf, u8 Len);
//     NextMCU_MsgDeal(RxBuf, Len);
// }
// #endif





// void parse_uart0_data(u8* RxBuf, u32 Len);
// void parse_uart1_data(u8* RxBuf, u32 Len);
// void parse_uart2_data(u8* RxBuf, u32 Len);















u8 uart2_data[512];
u8 uart1_data[512];
void master_led_flashing(void);
// void open_timer_test(void);
// static void open_timer_isr(void);
// void close_timer_test(void);
// static void close_timer_isr(void);


/**
 * @brief 找开机或关机的最大时长
 *
 * @param temp
 */
void find_max_time(ON_OFF_FLAG temp)
{
    u8 i = 0; // 循环计数值，注意该变量类型的大小不能小于时序器的继电器总数
    u32 time_cnt = 0; // 开机 

    /*
        将该时序器所有继电器的 开机 / 关机 时间累加，作为总 开机 / 关机 时间
    */
    if (temp == DEVICE_ON)
    {
        for (i = 0; i < sequencers.relay_number; i++)
        {
            time_cnt += sequencers.relay[i].open_time; // 累加所有继电器的 开机延时时间
        }

        sequencers.open_timeing = time_cnt;
    }
    else if (temp == DEVICE_OFF)
    {
        for (i = 0; i < sequencers.relay_number; i++)
        {
            time_cnt += sequencers.relay[i].close_time; // 累加所有继电器的 关机延时时间
        }

        sequencers.close_timeing = time_cnt;
    }
}



void fd_relay_state(void);


extern unsigned char voltage_array[3];
// extern unsigned char  power_array[4];

//功率计
// void parse_uart0_data(u8* RxBuf, u32 Len) // 分析串口 数据
// {


//     if (RxBuf[0] == 0x55 && RxBuf[1] == 0x5A && Len == 24) //&& sequencers.on_ff == DEVICE_ON)
//     {



//         DealUartInf(RxBuf, Len);

//     }
// }






/**
 * @brief 处理串口1接收到的数据
 *
 */
 // void parse_uart1_data(u8* RxBuf, u32 Len)
 // {
 //     u8 data_len = Len;
 //     memset(&uart1_data, 0, Len);
 //     memcpy(&uart1_data, RxBuf, Len);
 //     Uart2_Send_Tx(uart1_data, data_len);// 转发数据
 // }


 //上电初始化
void set_open_machine_flag(void)
{
}



#if 0
/*
*/
/**
 * @brief 串口指令解释 (解析串口2接收到的数据)
 *
 * @param RxBuf
 * @param Len
 */
void parse_uart2_data(u8* RxBuf, u32 Len)
{


    u8 data_len = Len;
    memset(&uart2_data, 0, Len);
    memcpy(&uart2_data, RxBuf, Len);
    u8 fb_information[30]; // 通过串口2 发送给PC或是上一级设备的信息
    u8 fb_uart1[30]; // 通过串口1发送给下一级设备的信息

    printf("sequencers.timeing_flag  = %d", sequencers.timeing_flag); // 打印当前设备的计时状态，是否在计时（处于开关机中）
    printf("sequencers.addr = %d", sequencers.addr); // 打印当前设备的地址

    // 所有指令在AD计时时，不执行下面的代码块
    if (sequencers.timeing_flag == 1)
    {

        // 设置开机时序
        if (uart2_data[0] == 0xFE && /* 帧头 */
            uart2_data[1] == 0x03 && /* 表示传输方向，上位机PC或是上一级设备 -> 当前设备 */
            uart2_data[2] == 0x04 && /* 控制命令 */
            (sequencers.addr == uart2_data[3] || 0x00 == uart2_data[3])) /* 设备地址 */
        {
            printf("【recv cmd】set open time \n");
            // printf("sequencers.on_ff = %d",sequencers.on_ff);
            // read_flash_sequencers_status_init();
            // printf("sequencers.on_ff = %d",sequencers.on_ff);
            sequencers.open_timeing = 0; // 清空总的开机时间
            /*
                给当前设备的每一路继电器都单独设置时序，

                例如，第一路继电器时序 == uart2_data[4]
                     第二路继电器时序 == uart2_data[5]
                     第三路继电器时序 == uart2_data[6]
            */
            for (u8 i = 4, j = 0; i <= (4 + RELAYS_MAX - 1); i++, j++) // 8路继电器 
            {
                printf("i : %d\n", i);
                sequencers.relay[j].open_time = uart2_data[i]; // 设置继电器对应的开机延时时间
                sequencers.relay[j].open_on_off = DEVICE_ON; // 设置继电器开机时，对应的状态

                sequencers.open_timeing += sequencers.relay[j].open_time; // 总开机时间累加
            }

            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE; /* 帧头 */
            fb_information[fb_info_len++] = 0X04; /* 传输方向 */
            fb_information[fb_info_len++] = 0x04; /* 命令 */
            fb_information[fb_info_len++] = sequencers.addr; /* 当前设备的地址 */
            fb_information[fb_info_len++] = sequencers.relay[0].open_time;
            fb_information[fb_info_len++] = sequencers.relay[1].open_time;
            fb_information[fb_info_len++] = sequencers.relay[2].open_time;
            fb_information[fb_info_len++] = sequencers.relay[3].open_time;
            fb_information[fb_info_len++] = sequencers.relay[4].open_time;
            fb_information[fb_info_len++] = sequencers.relay[5].open_time;
            fb_information[fb_info_len++] = sequencers.relay[6].open_time;
            fb_information[fb_info_len++] = sequencers.relay[7].open_time;
            fb_information[fb_info_len++] = 0xFF;
            Uart2_Send_Tx(fb_information, fb_info_len);  //返回
            // save_sequencers_data_area3();
            // save_user_data_area3();
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
        }

        // 2设置关机时序
        if (uart2_data[0] == 0xFE && /* 帧头 */
            uart2_data[1] == 0x03 && /* 表示传输方向，上位机PC或是上一级设备 -> 当前设备 */
            uart2_data[2] == 0x05 && /* 控制命令 */
            (sequencers.addr == uart2_data[3] || 0x00 == uart2_data[3])) /* 设备地址 */
        {
            printf("【recv cmd】set close time \n");
            sequencers.close_timeing = 0; // 清空总的关机时间

            /*
                给当前设备的每一路继电器都单独设置时序，

                例如，第一路继电器时序 == uart2_data[4]
                     第二路继电器时序 == uart2_data[5]
                     第三路继电器时序 == uart2_data[6]
            */
            for (int i = 4, j = 0; i <= (4 + RELAYS_MAX - 1); i++, j++)
            {
                printf("i : %d\n", i);
                sequencers.relay[j].close_time = uart2_data[i]; // 设置继电器对应的开机延时时间
                sequencers.relay[j].clod_on_off = DEVICE_OFF; // 设置继电器关机时，对应的状态

                sequencers.close_timeing += sequencers.relay[j].close_time; // 总关机时间累加
            }

            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE;
            fb_information[fb_info_len++] = 0X04;
            fb_information[fb_info_len++] = 0X05;
            fb_information[fb_info_len++] = sequencers.addr;
            fb_information[fb_info_len++] = sequencers.relay[0].close_time;
            fb_information[fb_info_len++] = sequencers.relay[1].close_time;
            fb_information[fb_info_len++] = sequencers.relay[2].close_time;
            fb_information[fb_info_len++] = sequencers.relay[3].close_time;
            fb_information[fb_info_len++] = sequencers.relay[4].close_time;
            fb_information[fb_info_len++] = sequencers.relay[5].close_time;
            fb_information[fb_info_len++] = sequencers.relay[6].close_time;
            fb_information[fb_info_len++] = sequencers.relay[7].close_time;
            fb_information[fb_info_len++] = 0xFF;
            Uart2_Send_Tx(fb_information, fb_info_len);  //返回
            // save_sequencers_data_area3();
            // save_user_data_area3();
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
        }

        // 3查看开机时序
        if (uart2_data[0] == 0xFE && /* 帧头 */
            uart2_data[1] == 0x03 && /* 表示传输方向，上位机PC或是上一级设备 -> 当前设备 */
            uart2_data[2] == 0x06 && /* 控制命令 */
            (sequencers.addr == uart2_data[3] || 0x00 == uart2_data[3])) /* 设备地址 */
        {
            printf("【recv cmd】get open time \n");
            save_user_data_init();
            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE; /* 帧头 */
            fb_information[fb_info_len++] = 0X04; /* 传输方向 */
            fb_information[fb_info_len++] = 0x04; /* 命令 */
            fb_information[fb_info_len++] = sequencers.addr; /* 当前设备的地址 */
            fb_information[fb_info_len++] = sequencers.relay[0].open_time;
            fb_information[fb_info_len++] = sequencers.relay[1].open_time;
            fb_information[fb_info_len++] = sequencers.relay[2].open_time;
            fb_information[fb_info_len++] = sequencers.relay[3].open_time;
            fb_information[fb_info_len++] = sequencers.relay[4].open_time;
            fb_information[fb_info_len++] = sequencers.relay[5].open_time;
            fb_information[fb_info_len++] = sequencers.relay[6].open_time;
            fb_information[fb_info_len++] = sequencers.relay[7].open_time;
            fb_information[fb_info_len++] = 0xFF;
            Uart2_Send_Tx(fb_information, fb_info_len);  //返回
        }


        // 4查看关机时序
        if (uart2_data[0] == 0xFE && /* 帧头 */
            uart2_data[1] == 0x03 && /* 表示传输方向，上位机PC或是上一级设备 -> 当前设备 */
            uart2_data[2] == 0x07 && /* 控制命令 */
            (sequencers.addr == uart2_data[3] || 0x00 == uart2_data[3])) /* 设备地址 */
        {
            printf("【recv cmd】get close time \n");
            save_user_data_init();
            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE; /* 帧头 */
            fb_information[fb_info_len++] = 0X04; /* 传输方向 */
            fb_information[fb_info_len++] = 0x04; /* 命令 */
            fb_information[fb_info_len++] = sequencers.addr; /* 当前设备的地址 */
            fb_information[fb_info_len++] = sequencers.relay[0].close_time;
            fb_information[fb_info_len++] = sequencers.relay[1].close_time;
            fb_information[fb_info_len++] = sequencers.relay[2].close_time;
            fb_information[fb_info_len++] = sequencers.relay[3].close_time;
            fb_information[fb_info_len++] = sequencers.relay[4].close_time;
            fb_information[fb_info_len++] = sequencers.relay[5].close_time;
            fb_information[fb_info_len++] = sequencers.relay[6].close_time;
            fb_information[fb_info_len++] = sequencers.relay[7].close_time;
            fb_information[fb_info_len++] = 0xFF;
            Uart2_Send_Tx(fb_information, fb_info_len);  //返回
        }


        // 查看当前设备继电器的开关状态
        if (uart2_data[0] == 0xFE && /* 帧头 */
            uart2_data[1] == 0x03 && /* 表示传输方向，上位机PC或是上一级设备 -> 当前设备 */
            uart2_data[2] == 0x08 && /* 控制命令 */
            (sequencers.addr == uart2_data[3] || 0x00 == uart2_data[3])) /* 设备地址 */
        {
            printf("【recv cmd】get status \n");
            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE; /* 帧头 */
            fb_information[fb_info_len++] = 0X04; /* 传输方向 */
            fb_information[fb_info_len++] = 0x04; /* 命令 */
            fb_information[fb_info_len++] = sequencers.addr; /* 当前设备的地址 */
            fb_information[fb_info_len++] = temp_on_off[0];
            fb_information[fb_info_len++] = temp_on_off[1];
            fb_information[fb_info_len++] = temp_on_off[2];
            fb_information[fb_info_len++] = temp_on_off[3];
            fb_information[fb_info_len++] = temp_on_off[4];
            fb_information[fb_info_len++] = temp_on_off[5];
            fb_information[fb_info_len++] = temp_on_off[6];
            fb_information[fb_info_len++] = temp_on_off[7];
            fb_information[fb_info_len++] = 0xFF;

            Uart2_Send_Tx(fb_information, (10 + sequencers.relay_number));
        }



        //地址不是本地设备，发送到级联设备
        if (uart2_data[2] != sequencers.addr && uart2_data[2] != 0)
        {
            Uart1_Send_Tx(uart2_data, data_len);
        }

    }  //时序计时时，所有指令不相应 
}


#endif
/**
 * @brief 设备第一次使用的初始化
 *
 */
void sequencers_data_init()
{
    printf("-------------------------------------------------sequencers_data_init");
    u8 open_set_cnt;
    u8 close_set_cnt;
    u8 i = RELAYS_MAX;
    sequencers.addr = 1; // 0：作用是：地址需要设置了才能用
    sequencers.on_ff = DEVICE_OFF; // 默认不要开机
    sequencers.relay_number = RELAYS_MAX; // 继电器数量
    // sequencers.timeing_flag = 1; // 表示时序器不处于开关机的延时状态
    // sequencers.open_timeing = 0; //默认设备开关机时序不计时 
    sequencer_flag_in_delay_clear(); // 默认设备不处于开关机的延时中

    //默认开机时逐个亮
    for (open_set_cnt = 0; open_set_cnt < RELAYS_MAX; open_set_cnt++)
    {
        // sequencers.relay[open_set_cnt].open_time = open_set_cnt + 1;

        // sequencers.relay[open_set_cnt].open_time = i;
        sequencers.relay[open_set_cnt].open_time = 1;
        // sequencers.relay[open_set_cnt].open_time = 0; // 测试，如果开机延时时间为0
    }

    //默认关机时逐个灭
    for (close_set_cnt = 0; close_set_cnt < RELAYS_MAX; close_set_cnt++)
    {
        // sequencers.relay[close_set_cnt].close_time = close_set_cnt + 1;
        sequencers.relay[close_set_cnt].close_time = 1;
    }

    for (u8 i = 0; i < RELAYS_MAX; i++)
    {
        sequencers.relay[i].last_status_on_off = DEVICE_ON; // 初始化，下次开机让所有继电器打开
        sequencers.relay[i].cur_status_on_off = DEVICE_OFF; // 继电器当前的初始状态
    }

    // find_max_time(DEVICE_ON);
    // find_max_time(DEVICE_OFF);
}

/**
 * @brief  APP指令控制继电器
 *
 * @param relay_led   AD按键 灯
 * @param le_state    继电器
 */
 // extern  ON_OFF_FLAG temp_on_off[16];  //继电器的开关

void relay_off_on(u32 relay_led, u8 relay_number)
{
    if (temp_on_off[relay_number] == DEVICE_ON)
    {
        gpio_direction_output(relay_led, 1); // 开灯
        lcd_show_relay_icon(relay_number); // lcd点亮对应的通道
    }
    else
    {
        gpio_direction_output(relay_led, 0); // 关灯
        lcd_clear_relay_icon(relay_number); // LCD清除单个继电器图标(不显示对应的继电器图标)
    }
}


void fd_relay_state(void)
{
}



/**
 * @brief AD按键控制继电器
 *
 */
void adkey_control(u32 relay_led, u8 relay_number)
{
    temp_on_off[relay_number] = !temp_on_off[relay_number];
    if (temp_on_off[relay_number] == DEVICE_ON)
    {
        gpio_direction_output(relay_led, 1); // 点亮对应按键的灯 同时也会打开对应的继电器
        lcd_show_relay_icon(relay_number); // lcd 对应的通道
    }
    else
    {
        gpio_direction_output(relay_led, 0); // 关闭按键对应的灯，同时关闭对应的继电器
        lcd_clear_relay_icon(relay_number); // lcd 对应的通道
    }

}






u16 timer_id = 0;                       // 定时器ID
u16 timer_cnt = 0;
int temp_time = 0;
// void need_handle_relays(ON_OFF_FLAG temp);

u8 delay_2s_close_f = 0;
/****************************************************************   关机  **************************************/
void all_shutdowm(void);


/**
 * @brief io 按键 ， 控制时序器 开关机
 *
 */
void iokey_master_on_off(void)
{
    printf("sequencers.on_ff  %u\n", (u16)sequencers.on_ff);

    // u8 next_data[7];
    // printf("io key_master_on_off\n");
    if (sequencers.on_ff == DEVICE_OFF)    // ---------------------- 开机
        // if (0 == flag_is_lcd_screen_on)
    {
        printf("io key_master_on_off   open\n");
        sequencer_power_on();
    }
    else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
        // else // 1 == flag_is_lcd_screen_on
    {
        printf("io key_master_on_off   off\n");
        sequencer_power_off();
    }
}

// ---------------------------------------------  控制面板的功能逻辑  ----------------------------------------------------

// #include "lcd1621.h"
extern u8 lcd_now_state;
u8 time_unit = 0;
u8 sys_time_unit = 0;
extern u8 blink_f;
u8 chose_relays_num = 0;
//使用数组的想法是，将8个继电器的临时时间分别存，这样可以不混乱，任意按键退出设置模式后，8路继电器都能保存

u8 split_open_time[8][4] = { 0 };
u8 split_close_time[8][4] = { 0 };
extern u16 blink_cnt;

// void make_lock_screen(void)
// {
//     clean_dis(clrbit(SEG_X3));
//     make_dis(SEG_X1);  //
//     make_dis(SEG_X2);

// }


// void dis_lock_screen(void)
// {
//     clean_dis(clrbit(SEG_X1));
//     make_dis(SEG_X2);  //
//     make_dis(SEG_X3);


// }


#define pre_tiem 20



extern u8 temp_year[4];
extern u8 temp_month[2];
extern u8 temp_day[2];
extern u8 temp_hour[2];
extern u8 temp_min[2];
extern u8 temp_sec[2];

u8 set_countdown_open_year[8][4] = { 0 };
u8 set_countdown_open_month[8][2] = { 0 };
u8 set_countdown_open_day[8][2] = { 0 };
u8 set_countdown_open_hour[8][2] = { 0 };
u8 set_countdown_open_min[8][2] = { 0 };
u8 set_countdown_open_sec[8][2] = { 0 };

u8 set_countdown_close_year[8][4] = { 0 };
u8 set_countdown_close_month[8][2] = { 0 };
u8 set_countdown_close_day[8][2] = { 0 };
u8 set_countdown_close_hour[8][2] = { 0 };
u8 set_countdown_close_min[8][2] = { 0 };
u8 set_countdown_close_sec[8][2] = { 0 };




u8 m_arry[8] = { 0,0,0,0,0,0,0,0 };
u8 show_e_f = 0; // 显示e还是显示f

// void sequencer_relay_status_update(relay_index_t relay_index , relay_status_t relay_status)




/**
 * @brief AD按键控制16路继电器
 *
 * @param keyevent    AD按键消息
 */
void ad_key_event_handle(int keyevent)
{
    switch (keyevent)
    {
        // USER_TO_DO 后续需要添加独立的开机和关机延时， 加上 232 的反馈信息
        //继电器                  //控灯                       向上位机反馈
    case KEY0_AD_CLICK: // 第一路对应的继电器按键 
    case KEY0_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_0);

        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================
    case KEY1_AD_CLICK:
    case KEY1_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_1);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;

    // ===================================================================
    case KEY2_AD_CLICK:
    case KEY2_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_2);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================
    case KEY3_AD_CLICK:
    case KEY3_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_3);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================
    case KEY4_AD_CLICK:
    case KEY4_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_4);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================
    case KEY5_AD_CLICK:
    case KEY5_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_5);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================
    case KEY6_AD_CLICK:
    case KEY6_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_6);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================
    case KEY7_AD_CLICK:
    case KEY7_AD_LONG:
    {
        sequencer_relay_status_toggle(RELAY_INDEX_7);
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    // ===================================================================

    default:
    {
        return;
    }
    break;

    }// switch (keyevent)


    os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
}

//将开机时拆分成分秒的格式
void split_open_minute_second()
{
    u8 i = 0;
    // read_flash_sequencers_status_init();
    for (i = 0; i < 8; i++)
    {
        split_open_time[i][0] = 0;
        split_open_time[i][1] = sequencers.relay[i].open_time / 60;
        split_open_time[i][2] = (sequencers.relay[i].open_time % 60) / 10;
        split_open_time[i][3] = (sequencers.relay[i].open_time % 60) % 10;
    }


}
//将关机时拆分成分秒的格式
void split_close_minute_second()
{
    u8 i = 0;
    // read_flash_sequencers_status_init();
    for (i = 0; i < 8; i++)
    {
        split_close_time[i][0] = 0;
        split_close_time[i][1] = sequencers.relay[i].close_time / 60;
        split_close_time[i][2] = (sequencers.relay[i].close_time % 60) / 10;
        split_close_time[i][3] = (sequencers.relay[i].close_time % 60) % 10;
    }
}







//将设置完成的8路开机时间，存在结构体中
void sum_open_minute_second(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.relay[i].open_time = (split_open_time[i][0] * 10 + split_open_time[i][1]) * 60 + (split_open_time[i][2] * 10 + split_open_time[i][3]);
    }
}


//将设置完成的8路关机时间，存在结构体中
void sum_close_minute_second(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.relay[i].close_time = (split_close_time[i][0] * 10 + split_close_time[i][1]) * 60 + (split_close_time[i][2] * 10 + split_close_time[i][3]);
    }
}






void read_relays_countdown_open_time(void)
{
    u8 i = 0;

    for (i = 0; i < 8; i++)
    {
        set_countdown_open_year[i][0] = sequencers.relay[i].countdown_open_time.year / 1000;
        set_countdown_open_year[i][1] = sequencers.relay[i].countdown_open_time.year % 1000 / 100;
        set_countdown_open_year[i][2] = sequencers.relay[i].countdown_open_time.year % 1000 % 100 / 10;
        set_countdown_open_year[i][3] = sequencers.relay[i].countdown_open_time.year % 1000 % 100 % 10;

        set_countdown_open_day[i][0] = sequencers.relay[i].countdown_open_time.day / 10;
        set_countdown_open_day[i][1] = sequencers.relay[i].countdown_open_time.day % 10;

        set_countdown_open_month[i][0] = sequencers.relay[i].countdown_open_time.month / 10;
        set_countdown_open_month[i][1] = sequencers.relay[i].countdown_open_time.month % 10;



        set_countdown_open_hour[i][0] = sequencers.relay[i].countdown_open_time.hour / 10;
        set_countdown_open_hour[i][1] = sequencers.relay[i].countdown_open_time.hour % 10;


        set_countdown_open_min[i][0] = sequencers.relay[i].countdown_open_time.min / 10;
        set_countdown_open_min[i][1] = sequencers.relay[i].countdown_open_time.min % 10;

        set_countdown_open_sec[i][0] = sequencers.relay[i].countdown_open_time.sec / 10;
        set_countdown_open_sec[i][1] = sequencers.relay[i].countdown_open_time.sec % 10;

    }



}

void read_relays_countdown_close_time(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        set_countdown_close_year[i][0] = sequencers.relay[i].countdown_close_time.year / 1000;
        set_countdown_close_year[i][1] = sequencers.relay[i].countdown_close_time.year % 1000 / 100;
        set_countdown_close_year[i][2] = sequencers.relay[i].countdown_close_time.year % 1000 % 100 / 10;
        set_countdown_close_year[i][3] = sequencers.relay[i].countdown_close_time.year % 1000 % 100 % 10;


        set_countdown_close_month[i][0] = sequencers.relay[i].countdown_close_time.month / 10;
        set_countdown_close_month[i][1] = sequencers.relay[i].countdown_close_time.month % 10;

        set_countdown_close_day[i][0] = sequencers.relay[i].countdown_close_time.day / 10;
        set_countdown_close_day[i][1] = sequencers.relay[i].countdown_close_time.day % 10;

        set_countdown_close_hour[i][0] = sequencers.relay[i].countdown_close_time.hour / 10;
        set_countdown_close_hour[i][1] = sequencers.relay[i].countdown_close_time.hour % 10;


        set_countdown_close_min[i][0] = sequencers.relay[i].countdown_close_time.min / 10;
        set_countdown_close_min[i][1] = sequencers.relay[i].countdown_close_time.min % 10;

        set_countdown_close_sec[i][0] = sequencers.relay[i].countdown_close_time.sec / 10;
        set_countdown_close_sec[i][1] = sequencers.relay[i].countdown_close_time.sec % 10;

    }
}


void write_relays_countdown_open_time(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.relay[i].countdown_open_time.year = set_countdown_open_year[i][0] * 1000 + set_countdown_open_year[i][1] * 100 + set_countdown_open_year[i][2] * 10 + set_countdown_open_year[i][3];
        sequencers.relay[i].countdown_open_time.month = set_countdown_open_month[i][0] * 10 + set_countdown_open_month[i][1];
        sequencers.relay[i].countdown_open_time.day = set_countdown_open_day[i][0] * 10 + set_countdown_open_day[i][1];
        sequencers.relay[i].countdown_open_time.hour = set_countdown_open_hour[i][0] * 10 + set_countdown_open_hour[i][1];
        sequencers.relay[i].countdown_open_time.min = set_countdown_open_min[i][0] * 10 + set_countdown_open_min[i][1];
        sequencers.relay[i].countdown_open_time.sec = set_countdown_open_sec[i][0] * 10 + set_countdown_open_sec[i][1];

    }

    // for(i = 0; i < 8; i++)
    // {
    //    printf("sequencers.relay[%d].countdown_open_time.year  = %d ",    i, sequencers.relay[i].countdown_open_time.year );
    //    printf("sequencers.relay[%d].countdown_open_time.month   = %d ",  i, sequencers.relay[i].countdown_open_time.month );
    //    printf("sequencers.relay[%d].countdown_open_time.day   = %d ",  i, sequencers.relay[i].countdown_open_time.day );
    //    printf("sequencers.relay[%d].countdown_open_time.hour   = %d ",   i, sequencers.relay[i].countdown_open_time.hour );
    //    printf("sequencers.relay[%d].countdown_open_time.min  = %d ",     i, sequencers.relay[i].countdown_open_time.min );
    //    printf("sequencers.relay[%d].countdown_open_time.sec  = %d ",     i, sequencers.relay[i].countdown_open_time.sec );

    // }
}

void write_relays_countdown_close_time(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.relay[i].countdown_close_time.year = set_countdown_close_year[i][0] * 1000 + set_countdown_close_year[i][1] * 100 + set_countdown_close_year[i][2] * 10 + set_countdown_close_year[i][3];
        sequencers.relay[i].countdown_close_time.month = set_countdown_close_month[i][0] * 10 + set_countdown_close_month[i][1];
        sequencers.relay[i].countdown_close_time.day = set_countdown_close_day[i][0] * 10 + set_countdown_close_day[i][1];
        sequencers.relay[i].countdown_close_time.hour = set_countdown_close_hour[i][0] * 10 + set_countdown_close_hour[i][1];
        sequencers.relay[i].countdown_close_time.min = set_countdown_close_min[i][0] * 10 + set_countdown_close_min[i][1];
        sequencers.relay[i].countdown_close_time.sec = set_countdown_close_sec[i][0] * 10 + set_countdown_close_sec[i][1];

    }


    // for(i = 0; i < 8; i++)
    // {
    //    printf("sequencers.relay[%d].countdown_close_time.year  = %d ",    i, sequencers.relay[i].countdown_close_time.year );
    //    printf("sequencers.relay[%d].countdown_close_time.month   = %d ",  i, sequencers.relay[i].countdown_close_time.month );
    //    printf("sequencers.relay[%d].countdown_close_time.day   = %d ",  i, sequencers.relay[i].countdown_close_time.day );
    //    printf("sequencers.relay[%d].countdown_close_time.hour   = %d ",   i, sequencers.relay[i].countdown_close_time.hour );
    //    printf("sequencers.relay[%d].countdown_close_time.min  = %d ",     i, sequencers.relay[i].countdown_close_time.min );
    //    printf("sequencers.relay[%d].countdown_close_time.sec  = %d ",     i, sequencers.relay[i].countdown_close_time.sec );

    // }

}







// u8 loc_screen_f = 0; // 标志位，是否有锁屏
extern u16 update_cnt;

/**
 * @brief 长按某个继电器按键 进入设置开关机延时的模式
 *
 * @param keyevent  按键键值消息
 */
void adkey_16way_long(int keyevent)
{

}


void all_shutdowm(void)
{
    u32 sw;
    for (u8 i = 0; i < RELAYS_MAX;i++)
    {
        temp_on_off[i] = DEVICE_OFF;
        sw = relay_table[i];
        relay_off_on(sw, i);
    }

}



/**
 * @brief 总开关灯闪烁
 *
 */
void master_led_flashing(void)
{
    static u8 sw0_led_flag = 0;  //作用：灯闪烁
    if (sw0_led_flag)
        gpio_direction_output(sw0_led, 1); //开灯
    else
        gpio_direction_output(sw0_led, 0); //关灯

    sw0_led_flag = !sw0_led_flag;
}


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
            // if (0 == flag_is_lcd_screen_on)
        {
            // 这里如果快速短按按键，会重复进入，目前在sequencer_power_on()内部判断是否处于延时，防止重复触发

            printf("ir key_master_on_off open\n");
            sequencer_power_on();
            return;
        }
        else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
            // else if (flag_is_lcd_screen_on)
        {
            // 这里如果快速短按按键，会重复进入，目前在sequencer_power_off()内部判断是否处于延时，防止重复触发
            printf("ir key_master_on_off off\n");
            sequencer_power_off();
            return;
        }
    }
    break;


    case KEY13_IR_CLICK:   //继电器1  USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw1_led, 0);

            // 继电器状态变化后，立刻更新继电器状态
            sequencers.relay[0].cur_status_on_off = temp_on_off[0];
            sequencers.relay[0].last_status_on_off = temp_on_off[0];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY14_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw2_led, 1);
            // 继电器状态变化后，立刻更新继电器状态
            sequencers.relay[1].cur_status_on_off = temp_on_off[1];
            sequencers.relay[1].last_status_on_off = temp_on_off[1];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY15_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw3_led, 2);   fd_relay_state();
            // 继电器状态变化后，立刻更新继电器状态
            sequencers.relay[2].cur_status_on_off = temp_on_off[2];
            sequencers.relay[2].last_status_on_off = temp_on_off[2];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY16_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw4_led, 3);   fd_relay_state();
            sequencers.relay[3].cur_status_on_off = temp_on_off[3];
            sequencers.relay[3].last_status_on_off = temp_on_off[3];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY17_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw5_led, 4);   fd_relay_state();
            sequencers.relay[4].cur_status_on_off = temp_on_off[4];
            sequencers.relay[4].last_status_on_off = temp_on_off[4];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY18_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw6_led, 5);   fd_relay_state();
            sequencers.relay[5].cur_status_on_off = temp_on_off[5];
            sequencers.relay[5].last_status_on_off = temp_on_off[5];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY19_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw7_led, 6);   fd_relay_state();
            sequencers.relay[6].cur_status_on_off = temp_on_off[6];
            sequencers.relay[6].last_status_on_off = temp_on_off[6];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY20_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw8_led, 7);   fd_relay_state();
            sequencers.relay[7].cur_status_on_off = temp_on_off[7];
            sequencers.relay[7].last_status_on_off = temp_on_off[7];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    } // switch (keyevent)





}





