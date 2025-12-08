#include "system/includes.h"
#include "system/event.h"
#include "includes.h"
#include "app_task.h"
#include "key_event_deal.h"

#include "../../apps/user_app/user_config.h"

// #include "../../apps/user_app/lcd/lcd1621.h"
#include "../../apps/user_app/sequencer/sequencer.h" // 时序器相关变量类型和变量定义
#include "../../apps/user_app/sequencer/sequencer_device_on_off.h" // 时序器设备开关控制
#include "../../apps/user_app/flash_handle/flash_handle.h" // flash读写接口




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
void parse_uart1_data(u8* RxBuf, u32 Len);
void parse_uart2_data(u8* RxBuf, u32 Len);















u8 uart2_data[512];
u8 uart1_data[512];
void master_led_flashing(void);
void open_timer_test(void);
static void open_timer_isr(void);
void close_timer_test(void);
static void close_timer_isr(void);


/**
 * @brief 找开机或关机的最大时长
 *
 * @param temp
 */
void find_max_time(ON_OFF_FLAG temp)
{
#if 0
    u8 i = 0;
    if (temp == DEVICE_ON)
    {
        for (i = 0; i < sequencers.relay_number; i++)
        {
            if (sequencers.realy[i].open_time > sequencers.open_timeing)
                sequencers.open_timeing = sequencers.realy[i].open_time;
        }
    }
    if (temp == DEVICE_OFF)
    {
        for (i = 0; i < sequencers.relay_number; i++)
        {
            if (sequencers.realy[i].close_time > sequencers.close_timeing)
                sequencers.close_timeing = sequencers.realy[i].close_time;
        }
    }
#endif

    u8 i = 0; // 循环计数值，注意该变量类型的大小不能小于时序器的继电器总数
    u16 time_cnt = 0; // 开机/

    /*
        将该时序器所有继电器的 开机 / 关机 时间累加，作为总 开机 / 关机 时间
    */
    if (temp == DEVICE_ON)
    {
        for (i = 0; i < sequencers.relay_number; i++)
        {
            time_cnt += sequencers.realy[i].open_time; // 累加所有继电器的 开机延时时间
        }

        sequencers.open_timeing = time_cnt;
    }
    else if (temp == DEVICE_OFF)
    {
        for (i = 0; i < sequencers.relay_number; i++)
        {
            time_cnt += sequencers.realy[i].close_time; // 累加所有继电器的 关机延时时间
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
void parse_uart1_data(u8* RxBuf, u32 Len)
{
    u8 data_len = Len;
    memset(&uart1_data, 0, Len);
    memcpy(&uart1_data, RxBuf, Len);
    Uart2_Send_Tx(uart1_data, data_len);// 转发数据
}


//上电初始化
void set_open_machine_flag(void)
{
}




/*
    USER_TO_DO 232RECV 232 RECV
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
                sequencers.realy[j].open_time = uart2_data[i]; // 设置继电器对应的开机延时时间
                sequencers.realy[j].open_on_off = DEVICE_ON; // 设置继电器开机时，对应的状态

                sequencers.open_timeing += sequencers.realy[j].open_time; // 总开机时间累加
            }

            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE; /* 帧头 */
            fb_information[fb_info_len++] = 0X04; /* 传输方向 */
            fb_information[fb_info_len++] = 0x04; /* 命令 */
            fb_information[fb_info_len++] = sequencers.addr; /* 当前设备的地址 */
            fb_information[fb_info_len++] = sequencers.realy[0].open_time;
            fb_information[fb_info_len++] = sequencers.realy[1].open_time;
            fb_information[fb_info_len++] = sequencers.realy[2].open_time;
            fb_information[fb_info_len++] = sequencers.realy[3].open_time;
            fb_information[fb_info_len++] = sequencers.realy[4].open_time;
            fb_information[fb_info_len++] = sequencers.realy[5].open_time;
            fb_information[fb_info_len++] = sequencers.realy[6].open_time;
            fb_information[fb_info_len++] = sequencers.realy[7].open_time;
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
                sequencers.realy[j].close_time = uart2_data[i]; // 设置继电器对应的开机延时时间
                sequencers.realy[j].clod_on_off = DEVICE_OFF; // 设置继电器关机时，对应的状态

                sequencers.close_timeing += sequencers.realy[j].close_time; // 总关机时间累加
            }

            u8 fb_info_len = 0;
            fb_information[fb_info_len++] = 0xFE;
            fb_information[fb_info_len++] = 0X04;
            fb_information[fb_info_len++] = 0X05;
            fb_information[fb_info_len++] = sequencers.addr;
            fb_information[fb_info_len++] = sequencers.realy[0].close_time;
            fb_information[fb_info_len++] = sequencers.realy[1].close_time;
            fb_information[fb_info_len++] = sequencers.realy[2].close_time;
            fb_information[fb_info_len++] = sequencers.realy[3].close_time;
            fb_information[fb_info_len++] = sequencers.realy[4].close_time;
            fb_information[fb_info_len++] = sequencers.realy[5].close_time;
            fb_information[fb_info_len++] = sequencers.realy[6].close_time;
            fb_information[fb_info_len++] = sequencers.realy[7].close_time;
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
            fb_information[fb_info_len++] = sequencers.realy[0].open_time;
            fb_information[fb_info_len++] = sequencers.realy[1].open_time;
            fb_information[fb_info_len++] = sequencers.realy[2].open_time;
            fb_information[fb_info_len++] = sequencers.realy[3].open_time;
            fb_information[fb_info_len++] = sequencers.realy[4].open_time;
            fb_information[fb_info_len++] = sequencers.realy[5].open_time;
            fb_information[fb_info_len++] = sequencers.realy[6].open_time;
            fb_information[fb_info_len++] = sequencers.realy[7].open_time;
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
            fb_information[fb_info_len++] = sequencers.realy[0].close_time;
            fb_information[fb_info_len++] = sequencers.realy[1].close_time;
            fb_information[fb_info_len++] = sequencers.realy[2].close_time;
            fb_information[fb_info_len++] = sequencers.realy[3].close_time;
            fb_information[fb_info_len++] = sequencers.realy[4].close_time;
            fb_information[fb_info_len++] = sequencers.realy[5].close_time;
            fb_information[fb_info_len++] = sequencers.realy[6].close_time;
            fb_information[fb_info_len++] = sequencers.realy[7].close_time;
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
    sequencers.timeing_flag = 1; // 表示时序器不处于开关机的延时状态
    // sequencers.open_timeing = 0; //默认设备开关机时序不计时 

    //默认开机时逐个亮
    for (open_set_cnt = 0; open_set_cnt < RELAYS_MAX; open_set_cnt++)
    {
        // sequencers.realy[open_set_cnt].open_time = open_set_cnt + 1;

        // sequencers.realy[open_set_cnt].open_time = i;
        sequencers.realy[open_set_cnt].open_time = 1;
        // sequencers.realy[open_set_cnt].open_time = 0; // 测试，如果开机延时时间为0
        sequencers.realy[open_set_cnt].open_on_off = DEVICE_ON; // 继电器开机时对应的状态

    }

    //默认关机时逐个灭
    for (close_set_cnt = 0; close_set_cnt < RELAYS_MAX; close_set_cnt++)
    {
        // sequencers.realy[close_set_cnt].close_time = close_set_cnt + 1;
        sequencers.realy[close_set_cnt].close_time = 1;

        // sequencers.realy[close_set_cnt].close_time = 0; // 测试，如果关机延时时间为0
        sequencers.realy[close_set_cnt].clod_on_off = DEVICE_OFF; // 继电器关机时对应的状态 
    }

    for (u8 i = 0; i < RELAYS_MAX; i++)
    {
        sequencers.realy[i].last_status_on_off = DEVICE_ON; // 初始化，下次开机让所有继电器打开
        sequencers.realy[i].cur_status_on_off = DEVICE_OFF; // 继电器当前的初始状态
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
void need_handle_relays(ON_OFF_FLAG temp);

u8 delay_2s_close_f = 0;
/****************************************************************   关机  **************************************/
void all_shutdowm(void);
/**
 * @brief 关机时序，关闭定时器
 *
 */
static void close_timer_isr(void)
{
    static u8 cur_relay_index = RELAYS_MAX - 1; // 时序器的继电器索引，关机时，默认从最后一个继电器开始关闭
    static u8 cur_relay_close_time_cnt = 0; // 当前继电器的关机延时时间计数

    static u8 delay_2s_cnt = 0; // 所有继电器关闭后，需要等2s，再关闭LCD显示和背光
    sequencers.timeing_flag = 0; // 标志位清零，表示关机时序执行中

    if (timer_cnt >= sequencers.close_timeing)  // 条件必须是有等于，可能关机时序时0秒 
    {
        all_shutdowm();  //确保所有继电器的状态是关机状态
        gpio_direction_output(sw0_led, 0); //关闭总开关的指示灯 

        //为了延时2s才关闭屏幕的
        delay_2s_cnt++;
        if (delay_2s_cnt == 4) //500*4 = 2000ms
        {
            delay_2s_cnt = 0;
            sys_s_hi_timer_del(timer_id);   // 注销定时器  停止计时
            timer_cnt = 0;
            timer_id = 0;                   // 防止重复注册

            // //关机，点亮三个mp3按键的灯
            // gpio_direction_output(IO_PORTA_11, 0);
            // gpio_direction_output(IO_PORTC_03, 0);
            // gpio_direction_output(IO_PORTC_02, 0);

            /*
                测试发现，在显示交流电电压界面的时候，按下关机，
                还是会有显示交流电电压，此时这个电压不会更新
                有概率关不掉交流电电压的显示，即使背光已经关闭，
            */
            // 清除显示的第 1 ~ 7位数字
            clean_num(1);clean_num(2);clean_num(3);
            clean_num(4);clean_num(5);clean_num(6); clean_num(7);
            clean_dis(SEG_S5); // 符号 V
            clean_dis(SEG_T1); // 继电器通道边框
            clean_dis(SEG_T);

            //关机 关闭 LCD屏的背光灯
            gpio_direction_output(IO_PORTA_07, 0);

            lcd1621_off();  //关闭lcd显示
            // printf("lcd1621 off\n"); 

            cur_relay_index = RELAYS_MAX - 1;
            cur_relay_close_time_cnt = 0;
            sequencers.on_ff = DEVICE_OFF;


            sequencers.timeing_flag = 1; // 表示时序执行完成 

            app_task_put_key_msg(APP_CMD, 0);  //推送按键消息
            app_task_switch_to(APP_SLEEP_TASK);
            return; // 提前退出，防止继续执行 master_led_flashing() 
        }

    }
    else
    {
        temp_time++;
        if ((temp_time %= 2) == 0)
        {
            timer_cnt++;
            cur_relay_close_time_cnt++;
        }

        if (cur_relay_close_time_cnt >= sequencers.realy[cur_relay_index].close_time &&
            (cur_relay_close_time_cnt > 0)) /* 关机时间不能为0，这里设置一个默认值 */
        {
            temp_on_off[cur_relay_index] = sequencers.realy[cur_relay_index].clod_on_off; // 关闭继电器
            relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关（函数内部会检查 temp_on_off 对应的状态）

            sequencers.realy[cur_relay_index].open_on_off = DEVICE_OFF; // 表示该继电器已经关闭

            cur_relay_close_time_cnt = 0;
            cur_relay_index--; // 从最后一个继电器，一直到第一个继电器
        }
    }

    master_led_flashing(); // 关机时序 总开门的闪烁 
}


/**
 * @brief 关机时序启动定时器
 *
 */
void close_timer_test(void)
{
    if (timer_id == 0)                  // 防止重复注册
    {
        timer_cnt = 0;
        delay_2s_close_f = 0;
        timer_id = sys_s_hi_timer_add(NULL, close_timer_isr, 500); // 注册定时器  500ms
    }
}


/***********************************************  开机 ***************************************************/
/**
 * @brief 开机时序，关闭定时器
 *
 */

static void open_timer_isr(void)
{
    static u8 cur_relay_index = 0; // 当前继电器索引
    static u8 cur_relay_open_time_cnt = 0; // 当前继电器的开启延时时间计数

    // 500ms进入一次
    sequencers.timeing_flag = 0;

    // printf("timer_cnt %u\n", timer_cnt);
    // printf("sequencers.open_timeing %u\n", (u16)sequencers.open_timeing);

    if (timer_cnt >= sequencers.open_timeing)// 条件必须是有等于，可能开机时序时0秒
    {
        sequencers.timeing_flag = 1;
        sys_s_hi_timer_del(timer_id);   // 注销定时器  停止计时
        timer_id = 0;                   // 防止重复注册

        // if (sequencers.timeing_flag == 1)
        // {
        gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）
        // clean_dis(clrbit(SEG_T));  //开机完后，关闭音符
        sequencers.on_ff = DEVICE_ON;
        // app_task_put_key_msg(APP_CMD, 0);  //推送按键消息

        timer_cnt = 0;
        cur_relay_index = 0;
        cur_relay_open_time_cnt = 0;
        printf("open machine\n");
        return; // 提前退出，防止继续执行 master_led_flashing() 
    }
    else
    {
        temp_time++;
        if ((temp_time %= 2) == 0) // 每一秒扫描所有继电器状态
        {
            timer_cnt++;
            cur_relay_open_time_cnt++; // 每过一秒，计数加一
        }

        // 如果关机前，对应的继电器就是关闭的，跳过该继电器
        // if (sequencers.realy[cur_relay_index].status_on_off == DEVICE_OFF)
        // {
        //     cur_relay_index++;
        // }

        if (cur_relay_open_time_cnt >= sequencers.realy[cur_relay_index].open_time &&
            (cur_relay_open_time_cnt > 0) /* 开机延时时间不能为0，这里强制设置一个默认值 */)
        {
            // 如果计时已经大于当前继电器的开机延时
            temp_on_off[cur_relay_index] = DEVICE_ON; // 继电器 开/关
            relay_off_on(relay_table[cur_relay_index], cur_relay_index); // 继电器对应的图标、按键灯的开关

            sequencers.realy[cur_relay_index].open_on_off = DEVICE_ON;

            cur_relay_open_time_cnt = 0;
            cur_relay_index++;
        }
    }

    master_led_flashing();  // 总开关灯闪
}

/**
 * @brief 开机时序启动定时器
 *
 */
void open_timer_test(void)
{
    if (timer_id == 0)                  // 防止重复注册
    {
        timer_cnt = 0;
        timer_id = sys_s_hi_timer_add(NULL, open_timer_isr, 500); // 注册定时器  500ms
    }
}


/**
 * @brief ad按键的总开关
 *
 */
void adkey_master_on_off(void)
{
    printf("sequencers.on_ff  %u\n", (u16)sequencers.on_ff);

    // u8 next_data[7];
    // printf("io key_master_on_off\n");
    if (sequencers.on_ff == DEVICE_OFF)    // ---------------------- 开机
        // if (0 == flag_is_lcd_screen_on)
    {
        printf("io key_master_on_off   open\n");
        //开机，点亮三个mp3按键的灯
        // gpio_direction_output(IO_PORTA_11, 1);
        // gpio_direction_output(IO_PORTC_03, 1);
        // gpio_direction_output(IO_PORTC_02, 1);

        //开机点亮LCD屏的背光灯
        // gpio_direction_output(lcd_light, 1);
        // //lcd屏幕显示轮廓
        // lcd_open_frame();
        // read_flash_sequencers_status_init();  //读取开机时序信息
        // find_max_time(DEVICE_ON);
        // open_timer_test();//开始时序

        // gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）

        // flag_is_lcd_screen_on = 1; // 表示lcd开启

        // // USER_TO_DO:
        // //实现一键开机
        // next_data[0] = 0xFE;
        // next_data[1] = 0x03;
        // next_data[2] = 0x00;
        // next_data[3] = 0x02;
        // next_data[4] = 0x01;  //开机
        // next_data[5] = 0xFF;
        // Uart1_Send_Tx(next_data, 6); //通过串口1发送给级联设备

        // //实现开启设备，软件界面变化
        // next_data[0] = 0xFE;
        // next_data[1] = 0X04;
        // next_data[2] = 0x01;
        // next_data[3] = 0x00;
        // next_data[4] = sequencers.addr;
        // next_data[5] = sequencers.relay_number;
        // next_data[6] = 0xFF;
        // Uart2_Send_Tx(next_data, 7);  //应答返回


        sequencer_power_on();
    }
    else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
        // else // 1 == flag_is_lcd_screen_on
    {
        printf("io key_master_on_off   off\n");

        // read_flash_sequencers_status_init();  //读取关机时序信息
        // find_max_time(DEVICE_OFF);
        // close_timer_test(); //关机时序

        // all_shutdowm();  // 确保所有继电器的状态是关机状态
        // gpio_direction_output(sw0_led, 0); //关闭总开关的指示灯 
        // // 清除显示的第 1 ~ 7位数字
        // clean_num(1);clean_num(2);clean_num(3);
        // clean_num(4);clean_num(5);clean_num(6); clean_num(7);
        // clean_dis(SEG_S5); // 符号 V
        // clean_dis(SEG_T1); // 继电器通道边框
        // clean_dis(SEG_T);

        // //关机 关闭 LCD屏的背光灯
        // gpio_direction_output(IO_PORTA_07, 0);

        // lcd1621_off();  //关闭lcd显示
        // flag_is_lcd_screen_on = 0; // 表示lcd关闭

        // make_dis(SEG_T);   // 音符

        // //实现一键关机
        // next_data[0] = 0xFE;
        // next_data[1] = 0x03;
        // next_data[2] = 0x00;
        // next_data[3] = 0x02;
        // next_data[4] = 0x00;  //关机
        // next_data[5] = 0xFF;
        // Uart1_Send_Tx(next_data, 6); //通过串口1发送给级联设备

        // //实现开启设备，软件界面变化
        // next_data[0] = 0xFE;
        // next_data[1] = 0X04;
        // next_data[2] = 0x01;
        // next_data[3] = 0x00;
        // next_data[4] = sequencers.addr;
        // next_data[5] = sequencers.relay_number;
        // next_data[6] = 0xFF;
        // Uart2_Send_Tx(next_data, 7);  //应答返回

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
        // fall through
    case KEY0_AD_LONG:
        // if (lcd_now_state == show_power) 
    {
        adkey_control(sw1_led, 0); // 继电器、继电器对应的按键灯和对应的LCD图标，状态取反

        // 更新状态：
        sequencers.realy[0].cur_status_on_off = temp_on_off[0];
        sequencers.realy[0].last_status_on_off = temp_on_off[0];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY1_AD_CLICK:
        // fall through
    case KEY1_AD_LONG:
    {
        adkey_control(sw2_led, 1); fd_relay_state();
        sequencers.realy[1].cur_status_on_off = temp_on_off[1];
        sequencers.realy[1].last_status_on_off = temp_on_off[1];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY2_AD_CLICK:
        // fall through
    case KEY2_AD_LONG:
    {
        adkey_control(sw3_led, 2); fd_relay_state();
        sequencers.realy[2].cur_status_on_off = temp_on_off[2];
        sequencers.realy[2].last_status_on_off = temp_on_off[2];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY3_AD_CLICK:
        // fall through
    case KEY3_AD_LONG:
    {
        adkey_control(sw4_led, 3); fd_relay_state();
        sequencers.realy[3].cur_status_on_off = temp_on_off[3];
        sequencers.realy[3].last_status_on_off = temp_on_off[3];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY4_AD_CLICK:
        // fall through
    case KEY4_AD_LONG:
    {
        adkey_control(sw5_led, 4); fd_relay_state();
        sequencers.realy[4].cur_status_on_off = temp_on_off[4];
        sequencers.realy[4].last_status_on_off = temp_on_off[4];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY5_AD_CLICK:
        // fall through
    case KEY5_AD_LONG:
    {
        adkey_control(sw6_led, 5); fd_relay_state();
        sequencers.realy[5].cur_status_on_off = temp_on_off[5];
        sequencers.realy[5].last_status_on_off = temp_on_off[5];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY6_AD_CLICK:
        // fall through
    case KEY6_AD_LONG:
    {
        adkey_control(sw7_led, 6); fd_relay_state();
        sequencers.realy[6].cur_status_on_off = temp_on_off[6];
        sequencers.realy[6].last_status_on_off = temp_on_off[6];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;
    case KEY7_AD_CLICK:
        // fall through
    case KEY7_AD_LONG:
    {
        adkey_control(sw8_led, 7); fd_relay_state();
        sequencers.realy[7].cur_status_on_off = temp_on_off[7];
        sequencers.realy[7].last_status_on_off = temp_on_off[7];
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }
    break;


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
        split_open_time[i][1] = sequencers.realy[i].open_time / 60;
        split_open_time[i][2] = (sequencers.realy[i].open_time % 60) / 10;
        split_open_time[i][3] = (sequencers.realy[i].open_time % 60) % 10;
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
        split_close_time[i][1] = sequencers.realy[i].close_time / 60;
        split_close_time[i][2] = (sequencers.realy[i].close_time % 60) / 10;
        split_close_time[i][3] = (sequencers.realy[i].close_time % 60) % 10;
    }
}







//将设置完成的8路开机时间，存在结构体中
void sum_open_minute_second(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.realy[i].open_time = (split_open_time[i][0] * 10 + split_open_time[i][1]) * 60 + (split_open_time[i][2] * 10 + split_open_time[i][3]);
    }
}


//将设置完成的8路关机时间，存在结构体中
void sum_close_minute_second(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.realy[i].close_time = (split_close_time[i][0] * 10 + split_close_time[i][1]) * 60 + (split_close_time[i][2] * 10 + split_close_time[i][3]);
    }
}






void read_relays_countdown_open_time(void)
{
    u8 i = 0;

    for (i = 0; i < 8; i++)
    {
        set_countdown_open_year[i][0] = sequencers.realy[i].countdown_open_time.year / 1000;
        set_countdown_open_year[i][1] = sequencers.realy[i].countdown_open_time.year % 1000 / 100;
        set_countdown_open_year[i][2] = sequencers.realy[i].countdown_open_time.year % 1000 % 100 / 10;
        set_countdown_open_year[i][3] = sequencers.realy[i].countdown_open_time.year % 1000 % 100 % 10;

        set_countdown_open_day[i][0] = sequencers.realy[i].countdown_open_time.day / 10;
        set_countdown_open_day[i][1] = sequencers.realy[i].countdown_open_time.day % 10;

        set_countdown_open_month[i][0] = sequencers.realy[i].countdown_open_time.month / 10;
        set_countdown_open_month[i][1] = sequencers.realy[i].countdown_open_time.month % 10;



        set_countdown_open_hour[i][0] = sequencers.realy[i].countdown_open_time.hour / 10;
        set_countdown_open_hour[i][1] = sequencers.realy[i].countdown_open_time.hour % 10;


        set_countdown_open_min[i][0] = sequencers.realy[i].countdown_open_time.min / 10;
        set_countdown_open_min[i][1] = sequencers.realy[i].countdown_open_time.min % 10;

        set_countdown_open_sec[i][0] = sequencers.realy[i].countdown_open_time.sec / 10;
        set_countdown_open_sec[i][1] = sequencers.realy[i].countdown_open_time.sec % 10;

    }



}

void read_relays_countdown_close_time(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        set_countdown_close_year[i][0] = sequencers.realy[i].countdown_close_time.year / 1000;
        set_countdown_close_year[i][1] = sequencers.realy[i].countdown_close_time.year % 1000 / 100;
        set_countdown_close_year[i][2] = sequencers.realy[i].countdown_close_time.year % 1000 % 100 / 10;
        set_countdown_close_year[i][3] = sequencers.realy[i].countdown_close_time.year % 1000 % 100 % 10;


        set_countdown_close_month[i][0] = sequencers.realy[i].countdown_close_time.month / 10;
        set_countdown_close_month[i][1] = sequencers.realy[i].countdown_close_time.month % 10;

        set_countdown_close_day[i][0] = sequencers.realy[i].countdown_close_time.day / 10;
        set_countdown_close_day[i][1] = sequencers.realy[i].countdown_close_time.day % 10;

        set_countdown_close_hour[i][0] = sequencers.realy[i].countdown_close_time.hour / 10;
        set_countdown_close_hour[i][1] = sequencers.realy[i].countdown_close_time.hour % 10;


        set_countdown_close_min[i][0] = sequencers.realy[i].countdown_close_time.min / 10;
        set_countdown_close_min[i][1] = sequencers.realy[i].countdown_close_time.min % 10;

        set_countdown_close_sec[i][0] = sequencers.realy[i].countdown_close_time.sec / 10;
        set_countdown_close_sec[i][1] = sequencers.realy[i].countdown_close_time.sec % 10;

    }
}


void write_relays_countdown_open_time(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.realy[i].countdown_open_time.year = set_countdown_open_year[i][0] * 1000 + set_countdown_open_year[i][1] * 100 + set_countdown_open_year[i][2] * 10 + set_countdown_open_year[i][3];
        sequencers.realy[i].countdown_open_time.month = set_countdown_open_month[i][0] * 10 + set_countdown_open_month[i][1];
        sequencers.realy[i].countdown_open_time.day = set_countdown_open_day[i][0] * 10 + set_countdown_open_day[i][1];
        sequencers.realy[i].countdown_open_time.hour = set_countdown_open_hour[i][0] * 10 + set_countdown_open_hour[i][1];
        sequencers.realy[i].countdown_open_time.min = set_countdown_open_min[i][0] * 10 + set_countdown_open_min[i][1];
        sequencers.realy[i].countdown_open_time.sec = set_countdown_open_sec[i][0] * 10 + set_countdown_open_sec[i][1];

    }

    // for(i = 0; i < 8; i++)
    // {
    //    printf("sequencers.realy[%d].countdown_open_time.year  = %d ",    i, sequencers.realy[i].countdown_open_time.year );
    //    printf("sequencers.realy[%d].countdown_open_time.month   = %d ",  i, sequencers.realy[i].countdown_open_time.month );
    //    printf("sequencers.realy[%d].countdown_open_time.day   = %d ",  i, sequencers.realy[i].countdown_open_time.day );
    //    printf("sequencers.realy[%d].countdown_open_time.hour   = %d ",   i, sequencers.realy[i].countdown_open_time.hour );
    //    printf("sequencers.realy[%d].countdown_open_time.min  = %d ",     i, sequencers.realy[i].countdown_open_time.min );
    //    printf("sequencers.realy[%d].countdown_open_time.sec  = %d ",     i, sequencers.realy[i].countdown_open_time.sec );

    // }
}

void write_relays_countdown_close_time(void)
{
    u8 i = 0;
    for (i = 0; i < 8; i++)
    {
        sequencers.realy[i].countdown_close_time.year = set_countdown_close_year[i][0] * 1000 + set_countdown_close_year[i][1] * 100 + set_countdown_close_year[i][2] * 10 + set_countdown_close_year[i][3];
        sequencers.realy[i].countdown_close_time.month = set_countdown_close_month[i][0] * 10 + set_countdown_close_month[i][1];
        sequencers.realy[i].countdown_close_time.day = set_countdown_close_day[i][0] * 10 + set_countdown_close_day[i][1];
        sequencers.realy[i].countdown_close_time.hour = set_countdown_close_hour[i][0] * 10 + set_countdown_close_hour[i][1];
        sequencers.realy[i].countdown_close_time.min = set_countdown_close_min[i][0] * 10 + set_countdown_close_min[i][1];
        sequencers.realy[i].countdown_close_time.sec = set_countdown_close_sec[i][0] * 10 + set_countdown_close_sec[i][1];

    }


    // for(i = 0; i < 8; i++)
    // {
    //    printf("sequencers.realy[%d].countdown_close_time.year  = %d ",    i, sequencers.realy[i].countdown_close_time.year );
    //    printf("sequencers.realy[%d].countdown_close_time.month   = %d ",  i, sequencers.realy[i].countdown_close_time.month );
    //    printf("sequencers.realy[%d].countdown_close_time.day   = %d ",  i, sequencers.realy[i].countdown_close_time.day );
    //    printf("sequencers.realy[%d].countdown_close_time.hour   = %d ",   i, sequencers.realy[i].countdown_close_time.hour );
    //    printf("sequencers.realy[%d].countdown_close_time.min  = %d ",     i, sequencers.realy[i].countdown_close_time.min );
    //    printf("sequencers.realy[%d].countdown_close_time.sec  = %d ",     i, sequencers.realy[i].countdown_close_time.sec );

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

/**
 * @brief 循环，查找需要操作的继电器
 *
 * @param temp
 */
 // extern ON_OFF_FLAG temp_on_off[16];

 // 继电器操作（开启、关闭）
void need_handle_relays(ON_OFF_FLAG temp)
{
    // printf("%s %d\n", __func__, __LINE__);

    u32 sw;
    if (temp == DEVICE_ON)   //开机
    {
        for (u8 i = 0; i < sequencers.relay_number;i++)
        {
            // printf("sequencers.realy[%d].open_on_off = %d", i,sequencers.realy[i].open_on_off);
            if (sequencers.realy[i].open_time != 0) // 开机时间不为0
            {
                if (sequencers.realy[i].open_time == timer_cnt && sequencers.realy[i].open_on_off == DEVICE_ON)
                {
                    temp_on_off[i] = sequencers.realy[i].open_on_off; // 继电器 开/关
                    sw = relay_table[i];
                    relay_off_on(sw, i); // 继电器对应的图标、按键灯的开关
                }
            }
        }
    }

    if (temp == DEVICE_OFF)   //关机
    {
        for (u8 i = 0; i < sequencers.relay_number;i++)
        {
            if (sequencers.realy[i].close_time != 0)
            {
                if (sequencers.realy[i].close_time == timer_cnt && sequencers.realy[i].clod_on_off == DEVICE_OFF)
                {
                    temp_on_off[i] = sequencers.realy[i].clod_on_off; // 继电器 开/关
                    sw = relay_table[i];
                    relay_off_on(sw, i);// 继电器对应的图标、按键灯的开关
                }
            }
        }

    }

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

            // //开机点亮LCD屏的背光灯
            // gpio_direction_output(lcd_light, 1);
            // //lcd屏幕显示轮廓
            // lcd_open_frame();
            // gpio_direction_output(sw0_led, 1); // 开灯（总开关对应的按键灯）

            // flag_is_lcd_screen_on = 1; // 表示lcd开启

            sequencer_power_on();
            return;
        }
        else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
            // else if (flag_is_lcd_screen_on)
        {
            // 这里如果快速短按按键，会重复进入，目前在sequencer_power_off()内部判断是否处于延时，防止重复触发
            printf("ir key_master_on_off off\n");
#if 0
            save_user_data_init();  //读取关机时序信息
            find_max_time(DEVICE_OFF);
            close_timer_test(); //关机时序
            make_dis(SEG_T);   // 音符
#endif

            // gpio_direction_output(sw0_led, 0); //关闭总开关的指示灯 
            // // 清除显示的第 1 ~ 7位数字
            // clean_num(1);clean_num(2);clean_num(3);
            // clean_num(4);clean_num(5);clean_num(6); clean_num(7);
            // clean_dis(SEG_S5); // 符号 V
            // clean_dis(SEG_T1); // 继电器通道边框
            // clean_dis(SEG_T);

            // //关机 关闭 LCD屏的背光灯
            // gpio_direction_output(IO_PORTA_07, 0);

            // lcd1621_off();  //关闭lcd显示
            // flag_is_lcd_screen_on = 0; // 表示lcd关闭

            sequencer_power_off();
            return;
        }
    }
    break;

    case KEY2_IR_CLICK:  //关机
        break;

    case KEY3_IR_CLICK:

        break;

    case KEY4_IR_CLICK:
        break;
        // case KEY7_IR_CLICK:  // 上一曲
        //     bt_key_music_prev();
        //     break;
        // case KEY9_IR_CLICK:  // 播放/暂停
        //     bt_key_music_pp();

        //     break;
        // case KEY8_IR_CLICK:  //下一曲
        //     bt_key_music_next();
        //     break;

    case KEY13_IR_CLICK:   //继电器1  USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw1_led, 0);

            // 继电器状态变化后，立刻更新继电器状态
            sequencers.realy[0].cur_status_on_off = temp_on_off[0];
            sequencers.realy[0].last_status_on_off = temp_on_off[0];
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
            sequencers.realy[1].cur_status_on_off = temp_on_off[1];
            sequencers.realy[1].last_status_on_off = temp_on_off[1];
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
            sequencers.realy[2].cur_status_on_off = temp_on_off[2];
            sequencers.realy[2].last_status_on_off = temp_on_off[2];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY16_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw4_led, 3);   fd_relay_state();
            sequencers.realy[3].cur_status_on_off = temp_on_off[3];
            sequencers.realy[3].last_status_on_off = temp_on_off[3];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY17_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw5_led, 4);   fd_relay_state();
            sequencers.realy[4].cur_status_on_off = temp_on_off[4];
            sequencers.realy[4].last_status_on_off = temp_on_off[4];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY18_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw6_led, 5);   fd_relay_state();
            sequencers.realy[5].cur_status_on_off = temp_on_off[5];
            sequencers.realy[5].last_status_on_off = temp_on_off[5];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY19_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw7_led, 6);   fd_relay_state();
            sequencers.realy[6].cur_status_on_off = temp_on_off[6];
            sequencers.realy[6].last_status_on_off = temp_on_off[6];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    case KEY20_IR_CLICK: // USER_TO_DO 后续需要添加独立的开机和关机延时
    {
        if (sequencers.on_ff == DEVICE_ON)
        {
            adkey_control(sw8_led, 7);   fd_relay_state();
            sequencers.realy[7].cur_status_on_off = temp_on_off[7];
            sequencers.realy[7].last_status_on_off = temp_on_off[7];
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 将保存数据的消息，发送给对应的线程
        }
    }
    break;
    } // switch (keyevent)





}

//仅在sleep调用
void irket_on_off(int keyevent)
{
    u8 next_data[6];
    printf("adkey_master_on_off   open");
    //开机，点亮三个mp3按键的灯
    gpio_direction_output(IO_PORTA_11, 1);
    gpio_direction_output(IO_PORTC_03, 1);
    gpio_direction_output(IO_PORTC_02, 1);

    //开机点亮LCD屏的背光灯
    gpio_direction_output(lcd_light, 1); //背光灯默认关

    //lcd屏幕显示轮廓
    lcd_open_frame();



    save_user_data_init();  //读取开机时序信息
    find_max_time(DEVICE_ON);
    open_timer_test();//开始时序

    //实现一键开机
    next_data[0] = 0xFE;
    next_data[1] = 0x03;
    next_data[2] = 0x00;
    next_data[3] = 0x02;
    next_data[4] = 0x01;  //开机
    next_data[5] = 0xFF;
    Uart1_Send_Tx(next_data, 6); //通过串口1发送给级联设备

    //实现开启设备，软件界面变化
    next_data[0] = 0xFE;
    next_data[1] = 0X04;
    next_data[2] = 0x01;
    next_data[3] = 0x00;
    next_data[4] = sequencers.addr;
    next_data[5] = sequencers.relay_number;
    next_data[6] = 0xFF;
    Uart2_Send_Tx(next_data, 7);  //应答返回

}


//music模式
void irkey_16way_click_music(int keyevent)
{
    // printf("%s \n =======================================\n", __func__); // 测试还会不会进入这里


    u8 next_data[7];
    switch (keyevent)
    {
    case KEY1_IR_CLICK:  //开机
        if (sequencers.on_ff == DEVICE_OFF)    // ---------------------- 开机
        {
            printf("adkey_master_on_off   open");
            //开机，点亮三个mp3按键的灯
            gpio_direction_output(IO_PORTA_11, 1);
            gpio_direction_output(IO_PORTC_03, 1);
            gpio_direction_output(IO_PORTC_02, 1);

            //开机点亮LCD屏的背光灯
            gpio_direction_output(lcd_light, 1); //背光灯默认关

            //lcd屏幕显示轮廓
            lcd_open_frame();



            save_user_data_init();  //读取开机时序信息
            find_max_time(DEVICE_ON);
            open_timer_test();//开始时序

            //实现一键开机
            next_data[0] = 0xFE;
            next_data[1] = 0x03;
            next_data[2] = 0x00;
            next_data[3] = 0x02;
            next_data[4] = 0x01;  //开机
            next_data[5] = 0xFF;
            Uart1_Send_Tx(next_data, 6); //通过串口1发送给级联设备

            //实现开启设备，软件界面变化
            next_data[0] = 0xFE;
            next_data[1] = 0X04;
            next_data[2] = 0x01;
            next_data[3] = 0x00;
            next_data[4] = sequencers.addr;
            next_data[5] = sequencers.relay_number;
            next_data[6] = 0xFF;
            Uart2_Send_Tx(next_data, 7);  //应答返回
        }
        else if (sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
        {
            printf("adkey_master_on_off   off");

            save_user_data_init();  //读取关机时序信息
            find_max_time(DEVICE_OFF);
            close_timer_test(); //关机时序
            make_dis(SEG_T);   // 音符
            //实现一键关机
            next_data[0] = 0xFE;
            next_data[1] = 0x03;
            next_data[2] = 0x00;
            next_data[3] = 0x02;
            next_data[4] = 0x00;  //关机
            next_data[5] = 0xFF;
            Uart1_Send_Tx(next_data, 6); //通过串口1发送给级联设备

            //实现开启设备，软件界面变化
            next_data[0] = 0xFE;
            next_data[1] = 0X04;
            next_data[2] = 0x01;
            next_data[3] = 0x00;
            next_data[4] = sequencers.addr;
            next_data[5] = sequencers.relay_number;
            next_data[6] = 0xFF;
            Uart2_Send_Tx(next_data, 7);  //应答返回
        }




        break;
    case KEY2_IR_CLICK:  //关机

        break;
    case KEY3_IR_CLICK:

        break;
    case KEY4_IR_CLICK:
        break;
    case KEY7_IR_CLICK:  // 上一曲
        app_task_put_key_msg(KEY8_AD_CLICK, 0);  //推送按键消息
        break;
    case KEY9_IR_CLICK:  // 播放/暂停
        app_task_put_key_msg(KEY9_AD_CLICK, 0);  //推送按键消息
        break;
    case KEY8_IR_CLICK:  //下一曲
        app_task_put_key_msg(KEY10_AD_CLICK, 0);  //推送按键消息
        break;

    case KEY13_IR_CLICK:   //继电器1
        adkey_control(sw1_led, 0);   fd_relay_state();
        break;
    case KEY14_IR_CLICK:
        adkey_control(sw2_led, 1);   fd_relay_state();
        break;
    case KEY15_IR_CLICK:
        adkey_control(sw3_led, 2);   fd_relay_state();
        break;
    case KEY16_IR_CLICK:
        adkey_control(sw4_led, 3);   fd_relay_state();
        break;
    case KEY17_IR_CLICK:
        adkey_control(sw5_led, 4);   fd_relay_state();
        break;
    case KEY18_IR_CLICK:
        adkey_control(sw6_led, 5);   fd_relay_state();
        break;
    case KEY19_IR_CLICK:
        adkey_control(sw7_led, 6);   fd_relay_state();
        break;
    case KEY20_IR_CLICK:
        adkey_control(sw8_led, 7);   fd_relay_state();
        break;




    }

}







