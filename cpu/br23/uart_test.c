#include "system/includes.h"
#include "system/event.h"
#include "includes.h"
#include "app_task.h"
#include "key_event_deal.h"

#define UART_DEV_USAGE_TEST_SEL         1       //uart_dev.c api接口使用方法选择
//  选择1  串口中断回调函数推送事件，由事件响应函数接收串口数据
//  选择2  由task接收串口数据

#define UART_DEV_TEST_MULTI_BYTE        1       //uart_dev.c 读写多个字节api / 读写1个字节api 选择

#define UART_RXBUF_SIZE             512
#define UART_CBUF_SIZE              512
#define UART_TIMEROUT               50

#define UART_EN_NUM                 2

//串口0
#define UART0_TX_PORT               IO_PORTB_04
#define UART0_RX_PORT               IO_PORTB_06

#define DEVICE_EVENT_FROM_UART0_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('4' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART0_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('5' << 8) | '\0')

static u8 uart0_cbuf[512] __attribute__((aligned(4)));
static u8 uart0_rxbuf[512] __attribute__((aligned(4)));
const uart_bus_t *uart0_bus = NULL;
static volatile uint32_t uart0_rcv_len = 0;
static volatile uint8_t uart0_rcv_flag = 1;

//end

//串口1
#define UART1_TX_PORT               IO_PORTC_00
#define UART1_RX_PORT               IO_PORTC_01
#define UART_BAUDRATE               9600

static u8 uart1_cbuf[512] __attribute__((aligned(4)));
static u8 uart1_rxbuf[512] __attribute__((aligned(4)));
const uart_bus_t *uart1_bus = NULL;
static volatile uint32_t uart1_rcv_len = 0;
static volatile uint8_t uart1_rcv_flag = 1;
// end

//串口2
#if UART_EN_NUM == 2
#define UART2_TX_PORT               IO_PORTA_09
#define UART2_RX_PORT               IO_PORTA_10

#define DEVICE_EVENT_FROM_UART2_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('2' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART2_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('3' << 8) | '\0')

static u8 uart2_cbuf[512] __attribute__((aligned(4)));
static u8 uart2_rxbuf[512] __attribute__((aligned(4)));
const uart_bus_t *uart2_bus = NULL;
static volatile uint32_t uart2_rcv_len = 0;
static volatile uint8_t uart2_rcv_flag = 1;
#endif
//end



// -------------------------------------- 时序器功能 ---------------------------------


#include "adkey.h"
#include "lcd1621.h"
extern void adkey_ctrl_lcd_relays_close(u8 relay_number);

SEQUENCER  sequencers;
extern u8 display_data[16];   //lcd数据
const u8 relay_table[RELAYS_MAX]= {
    //按键灯（继电器

    [0] =  sw1_led,
    [1] =  sw2_led,
    [2] =  sw3_led,
    [3] =  sw4_led,
    [4] =  sw5_led,
    [5] =  sw6_led,
    [6] =  sw7_led,
    [7] =  sw8_led,


};

static void uart_event_handler(struct sys_event *e);

///Uart TX 写入
void Uart0_Send_Tx(u8 *txBuf, u8 len)
{
    if (uart0_bus)
    {
        uart0_rcv_flag = 0;
        uart0_bus->write(txBuf, len);
        // printf_buf(txBuf,len);
    }
}


///Uart TX 写入
void Uart1_Send_Tx(u8 *txBuf, u8 len)
{
    if (uart1_bus)
    {
        uart1_rcv_flag = 0;
        uart1_bus->write(txBuf, len);
        // printf_buf(txBuf,len);
    }
}


void Uart2_Send_Tx(u8 *txBuf, u8 len)
{
#if UART_EN_NUM == 2
    if (uart2_bus)
    {
        uart2_rcv_flag = 0;
        uart2_bus->write(txBuf, len);
        // printf("uart2 write");
        // printf_buf(txBuf,len);
    }
#endif
}





/**
 * @brief 本地设备串口信息清零
 *
 * @param RxBuf
 * @param Len
 */
void Controller_MsgDeal(u8 *RxBuf, u8 Len)
{
    memset(RxBuf,0,Len);

}
/**
 * @brief 级联设备串口信息清零
 *
 * @param RxBuf
 * @param Len
 */
void NextMCU_MsgDeal(u8 *RxBuf, u8 Len)
{
 memset(RxBuf,0,Len);

}

/**
 * @brief 串口0接受数据清零
 *
 * @param RxBuf
 * @param Len
 */
void Uart0_Rx_Deal(u8 *RxBuf, u8 Len)
{
    extern void Controller_MsgDeal(u8 *RxBuf, u8 Len);
    Controller_MsgDeal(RxBuf, Len);
}

/**
 * @brief 串口1接受数据清零
 *
 * @param RxBuf
 * @param Len
 */
void Uart1_Rx_Deal(u8 *RxBuf, u8 Len)
{
    extern void Controller_MsgDeal(u8 *RxBuf, u8 Len);
    Controller_MsgDeal(RxBuf, Len);
}

#if UART_EN_NUM == 2
/**
 * @brief 串口2接受数据清零
 *
 * @param RxBuf
 * @param Len
 */
void Uart2_Rx_Deal(u8 *RxBuf, u8 Len)
{
    extern void NextMCU_MsgDeal(u8 *RxBuf, u8 Len);
    NextMCU_MsgDeal(RxBuf, Len);
}
#endif





void parse_uart0_data(u8 *RxBuf, u32 Len);
void parse_uart1_data(u8 *RxBuf, u32 Len);
void parse_uart2_data(u8 *RxBuf, u32 Len);



/**
 * @brief  串口消息处理 放在系统线程
 *
 * @param e
 */
static void uart_event_handler(struct sys_event *e)
{
    const uart_bus_t *uart1_bus;
    u32 uart_rxcnt = 0;

//串口0 耀祥时序器的功率计
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART0_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: DEVICE_EVENT_FROM_UART0_RX_OVERFLOW\n");
            uart0_bus = (const uart_bus_t *)e->u.dev.value;
            uart0_rcv_len = uart0_bus->read(uart0_rxbuf, sizeof(uart0_rxbuf), 0);

            printf_buf(uart0_rxbuf,uart0_rcv_len);

            parse_uart0_data(uart0_rxbuf,uart0_rcv_len);
    //    Uart1_Send_Tx(uart0_rxbuf,uart0_rcv_len);

            if (uart0_rcv_len) {
                uart0_rcv_flag = 1;

                Uart0_Rx_Deal(uart0_rxbuf, uart0_rcv_len);
            }

        }
    }
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART0_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event:DEVICE_EVENT_FROM_UART0_RX_OUTTIME\n");
            uart2_bus = (const uart_bus_t *)e->u.dev.value;
            uart0_rcv_len = uart2_bus->read(uart0_rxbuf, sizeof(uart0_rxbuf), 0);

            printf_buf(uart0_rxbuf,uart0_rcv_len);

            //  Uart1_Send_Tx(uart0_rxbuf,uart0_rcv_len);

            parse_uart0_data(uart0_rxbuf,uart0_rcv_len);

            if (uart0_rcv_len) {
                uart0_rcv_flag = 1;

                Uart0_Rx_Deal(uart0_rxbuf, uart0_rcv_len);
            }

        }
    }
//串口0 end

//串口1  发向耀祥时序器的级联设备
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: DEVICE_EVENT_FROM_UART_RX_OVERFLOW\n");
            uart1_bus = (const uart_bus_t *)e->u.dev.value;
            uart1_rcv_len = uart1_bus->read(uart1_rxbuf, sizeof(uart1_rxbuf), 0);   //接受到字符串实际长度
            printf_buf(uart1_rxbuf,uart1_rcv_len);
            parse_uart1_data(uart1_rxbuf, uart1_rcv_len);

            //清空串口内容
            if (uart1_rcv_len) {
                uart1_rcv_flag = 1;
                // uart1_bus->write(uart1_rxbuf, uart1_rcv_len);
                // Controller Msg
                Uart1_Rx_Deal(uart1_rxbuf, uart1_rcv_len);
            }

        }
    }
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event:DEVICE_EVENT_FROM_UART_RX_OUTTIME\n");
            uart1_bus = (const uart_bus_t *)e->u.dev.value;
            uart1_rcv_len = uart1_bus->read(uart1_rxbuf, sizeof(uart1_rxbuf), 0);
            printf_buf(uart1_rxbuf,uart1_rcv_len);
          parse_uart1_data(uart1_rxbuf, uart1_rcv_len);
            //接受完数据就清
            if (uart1_rcv_len) {
                uart1_rcv_flag = 1;
                // uart1_bus->write(uart1_rxbuf, uart1_rcv_len);
                // Controller Msg
                Uart1_Rx_Deal(uart1_rxbuf, uart1_rcv_len);
            }

        }
    }
    //串口1 end

    //串口2  耀祥时序器设备与PC端通信
#if UART_EN_NUM == 2
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART2_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: DEVICE_EVENT_FROM_UART_RX_OVERFLOW\n");
            uart2_bus = (const uart_bus_t *)e->u.dev.value;
            uart2_rcv_len = uart2_bus->read(uart2_rxbuf, sizeof(uart2_rxbuf), 0);
            printf_buf(uart2_rxbuf,uart2_rcv_len);
            if (uart2_rcv_len) {
                uart2_rcv_flag = 1;
                // uart2_bus->write(uart2_rxbuf, uart2_rcv_len);
                // NextMCU Msg
                Uart2_Rx_Deal(uart2_rxbuf, uart2_rcv_len);
            }

        }
    }
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART2_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event:DEVICE_EVENT_FROM_UART_RX_OUTTIME\n");
            uart2_bus = (const uart_bus_t *)e->u.dev.value;
            uart2_rcv_len = uart2_bus->read(uart2_rxbuf, sizeof(uart2_rxbuf), 0);
            printf_buf(uart2_rxbuf,uart2_rcv_len);
            parse_uart2_data(uart2_rxbuf, uart2_rcv_len);

            if (uart2_rcv_len) {
                uart2_rcv_flag = 1;
                // uart2_bus->write(uart2_rxbuf, uart2_rcv_len);
                // NextMCU Msg
                Uart2_Rx_Deal(uart2_rxbuf, uart2_rcv_len);
            }

        }
    }
    // app_task_put_usr_msg(APP_MSG_SYS_EVENT,0);
#endif

//串口2 end
}

SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_event_handler, 0);   //线程





/**
 * @brief 耀祥时序器串口0中断函数
 *
 * @param arg
 * @param status
 */
static void Uart0_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("Uart0_isr_hook\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART0_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart0_rx_ot_isr   2\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART0_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }

}


/**
 * @brief 耀祥时序器串口1中断函数
 *
 * @param arg
 * @param status
 */
static void Uart1_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;
    printf("Uart1_isr_hook");
    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("uart1_rx_isr\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart1_rx_ot_isr  1\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
}




#if UART_EN_NUM == 2
/**
 * @brief 耀祥时序器串口2中断函数
 *
 * @param arg
 * @param status
 */
static void Uart2_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("Uart2_isr_hook\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART2_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart2_rx_ot_isr   2\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART2_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
}
#endif





/**
 * @brief 耀祥时序器串口0初始化    接功率计
 *
 * @return u8
 */
u8 Uart0_Init(void)
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = UART0_TX_PORT;
    u_arg.rx_pin = UART0_RX_PORT;
    u_arg.rx_cbuf = uart0_cbuf;
    u_arg.rx_cbuf_size = UART_CBUF_SIZE;
    u_arg.frame_length = UART_RXBUF_SIZE;
    u_arg.rx_timeout = UART_TIMEROUT;
    u_arg.isr_cbfun = Uart0_isr_hook;
    // u_arg.baud = UART_BUADRATE;
    u_arg.baud = 4800;
    u_arg.is_9bit = 0;

    uart0_bus = uart_dev_open(&u_arg);
    if (uart0_bus != NULL)
    {
        gpio_set_pull_up(u_arg.rx_pin, 1);
        return 1;
    }
    return 0;
}


/**
 * @brief 耀祥时序器串口1初始化
 *
 * @return u8
 */
u8 Uart1_Init(void)
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = UART1_TX_PORT;
    u_arg.rx_pin = UART1_RX_PORT;
    u_arg.rx_cbuf = uart1_cbuf;
    u_arg.rx_cbuf_size = UART_CBUF_SIZE;
    u_arg.frame_length = UART_RXBUF_SIZE;
    u_arg.rx_timeout = UART_TIMEROUT;
    u_arg.isr_cbfun = Uart1_isr_hook;
    u_arg.baud = UART_BAUDRATE;
    u_arg.is_9bit = 0;

    uart1_bus = uart_dev_open(&u_arg);
    if (uart1_bus != NULL)
    {
        gpio_set_pull_up(u_arg.rx_pin, 1);
        return 1;
    }
    return 0;
}
/**
 * @brief 耀祥时序器串口2初始化
 * 与PC通信使用的串口2
 * @return u8
 */
u8 Uart2_Init(void)
{
#if UART_EN_NUM == 2
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = UART2_TX_PORT;
    u_arg.rx_pin = UART2_RX_PORT;
    u_arg.rx_cbuf = uart2_cbuf;
    u_arg.rx_cbuf_size = UART_CBUF_SIZE;
    u_arg.frame_length = UART_RXBUF_SIZE;
    u_arg.rx_timeout = UART_TIMEROUT;
    u_arg.isr_cbfun = Uart2_isr_hook;
    u_arg.baud = UART_BAUDRATE;
    u_arg.is_9bit = 0;

printf("Uart2_Init");

    uart2_bus = uart_dev_open(&u_arg);
    if (uart2_bus != NULL)
    {
        gpio_set_pull_up(u_arg.rx_pin, 1);
        return 1;
    }
    return 0;
#endif
}
/**
 * @brief 博朗时序器关闭串口
 *
 */
void Uart_Off(void)
{
    if (uart0_bus != NULL) {
        uart_dev_close(uart0_bus);
    }

    if (uart1_bus != NULL) {
        uart_dev_close(uart1_bus);
    }
#if UART_EN_NUM == 2
    if (uart2_bus != NULL) {
        uart_dev_close(uart2_bus);
    }
#endif
}


u8 uart2_data[512] ;
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
    u8 i = 0;
    if(temp == DEVICE_ON)
    {
        for(i = 0; i < sequencers.relay_number; i++)
        {
            if(sequencers.realy[i].open_time > sequencers.open_timeing)
            sequencers.open_timeing = sequencers.realy[i].open_time;
        }
    }
    if(temp == DEVICE_OFF )
    {

        for(i = 0; i < sequencers.relay_number; i++)
        {
            if(sequencers.realy[i].close_time > sequencers.close_timeing)
            sequencers.close_timeing = sequencers.realy[i].close_time;
        }
    }
}




void fd_relay_state(void);

extern void relay_off_on(u32 relay_led,u8 relay_number);
extern unsigned char voltage_array[3] ;
extern unsigned char  power_array[4] ;

//功率计
void parse_uart0_data(u8 *RxBuf, u32 Len)
{


    if(RxBuf[0] == 0x55 && RxBuf[1] == 0x5A && Len == 24) //&& sequencers.on_ff == DEVICE_ON)
    {



        DealUartInf(RxBuf,Len);

    //    clean_num(1);clean_num(2);clean_num(3);   //清
	// 	clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏


    }
}






/**
 * @brief 串口1测试
 *
 */
void parse_uart1_data(u8 *RxBuf, u32 Len)
{
    u8 data_len = Len;
    memset(&uart1_data,0,Len);
    memcpy(&uart1_data,RxBuf,Len);
    Uart2_Send_Tx(uart1_data,data_len);
}


//上电初始化
void set_open_machine_flag(void)
{
    sequencers.timeing_flag = 1;
    sequencers.on_ff = DEVICE_OFF;//上电默认关机
}


 ON_OFF_FLAG temp_on_off[16];  //继电器的开关

/**
 * @brief 串口指令解释
 *
 * @param RxBuf
 * @param Len
 */
void parse_uart2_data(u8 *RxBuf, u32 Len)
{

    u8 data_len = Len;
    memset(&uart2_data,0,Len);
    memcpy(&uart2_data,RxBuf,Len);
    u8 fb_information[30];
    u8 fb_uart1[30];

    printf("sequencers.timeing_flag  = %d",sequencers.timeing_flag);
    printf("sequencers.addr = %d", sequencers.addr);
    //开关设备  无条件转发给级联设备
    if(uart2_data[0] == 0xFE && uart2_data[1] == 0x03 && uart2_data[2] == 0x00 && uart2_data[3] == 0x02 && data_len == 6)
    {
        if(uart2_data[4] == 0x01 && sequencers.timeing_flag == 1 && sequencers.on_ff == DEVICE_OFF)   //开机
        {
            printf("open");
            sequencers.timeing_flag = 0;
            read_flash_sequencers_status_init();

            for(int i = 0; i < 16; i++)
            printf("sequencers.realy[%d].open_on_off = %d", i,sequencers.realy[i].open_on_off);


            //开机，点亮三个mp3按键的灯
            gpio_direction_output(IO_PORTA_11,1);
            gpio_direction_output(IO_PORTC_03,1);
            gpio_direction_output(IO_PORTC_02,1);

            //开机点亮LCD屏的背光灯
            gpio_direction_output(lcd_light, 1); //背光灯默认关

            //lcd屏幕显示轮廓
            lcd_open_frame();


            find_max_time(DEVICE_ON);
            open_timer_test();          //开始时序

            //实现开启设备，软件界面变化
            fb_information[0] = 0xFE;
            fb_information[1] = 0X04;
            fb_information[2] = 0x01;
            fb_information[3] = 0x00;
            fb_information[4] = sequencers.addr;
            fb_information[5] = sequencers.relay_number;
            fb_information[6] = 0xFF;
            Uart2_Send_Tx(fb_information, 7);  //应答返回


            Uart1_Send_Tx(uart2_data,data_len); //通过串口1发送给级联设备

            app_task_switch_to(APP_BT_TASK);
        }
        else if(uart2_data[4] == 0x00 && sequencers.timeing_flag == 1 && sequencers.on_ff == DEVICE_ON )
        {
            printf("close");
            sequencers.timeing_flag = 0;
            read_flash_sequencers_status_init();
            find_max_time(DEVICE_OFF);
            close_timer_test(); //关机时序
            make_dis(SEG_T);   // 音符
            //实现开启设备，软件界面变化
            fb_information[0] = 0xFE;
            fb_information[1] = 0X04;
            fb_information[2] = 0x01;
            fb_information[3] = 0x00;
            fb_information[4] = sequencers.addr;
            fb_information[5] = sequencers.relay_number;
            fb_information[6] = 0xFF;
            Uart2_Send_Tx(fb_information, 7);  //应答返回


            Uart1_Send_Tx(uart2_data,data_len);

        }
    }

    if(sequencers.timeing_flag == 1)  //所有指令在AD计时时，不执行
    {
         //设置地址  不需要转发
        if(uart2_data[0] == 0xFE && uart2_data[1] == 0x04 && uart2_data[2] == 0x00  && uart2_data[3] == 0x01 )
        {

            printf("set address");
            u8 next_address = 0;
            u8 fb_uart1[7];
            sequencers.addr =  uart2_data[4];
            sequencers.relay_number =  uart2_data[5];
            next_address = uart2_data[4] + 1;
            if(sequencers.addr  ==  255)
                next_address = 255;
            fb_information[0] = 0xFE;
            fb_information[1] = 0X04;  //指令长度
            fb_information[2] = 0x01;
            fb_information[3] = 0x01;
            fb_information[4] = sequencers.addr;
            fb_information[5] = sequencers.relay_number;
            fb_information[6] = 0xFF;
            Uart2_Send_Tx(fb_information, 7);  //发给串口2  返回PC段 固定7个字节  应答返回

           //级联
            fb_uart1[0] = 0xFE;
            fb_uart1[1] = 0x04;
            fb_uart1[2] = 0x00;
            fb_uart1[3] = 0x01;
            fb_uart1[4] = next_address;
            fb_uart1[5] = sequencers.relay_number;
            fb_uart1[6] = 0xFF;
            Uart1_Send_Tx(fb_uart1, 7);  //发送级联设备地址
            for(int i = 0; i < 16; i++)
            printf("sequencers.realy[%d].open_on_off = %d", i,sequencers.realy[i].open_on_off);
            printf("sequencers.addr = %d next_address = %d " ,sequencers.addr ,next_address);
            save_sequencers_data_area3();

        }

        //查看本地地址   //不需要转发
        if(uart2_data[0] == 0xFE && uart2_data[1] == 0x03 && uart2_data[2] == 0x00 && uart2_data[3] == 0x00 && uart2_data[4] == 0x00 && uart2_data[5] == 0xFF )
        {
            printf("check address");
            fb_information[0] = 0xFE;
            fb_information[1] = 0X04;
            fb_information[2] = 0x01;
            fb_information[3] = 0x00;
            fb_information[4] = sequencers.addr;
            fb_information[5] = sequencers.relay_number;
            fb_information[6] = 0xFF;
            Uart2_Send_Tx(fb_information, 7);  //应答返回
        }

        //1设置开机时序
        if(uart2_data[0] == 0xFE  && sequencers.addr == uart2_data[2] && uart2_data[3] == 0x04  )
        {
            printf("set open time ");
            // printf("sequencers.on_ff = %d",sequencers.on_ff);
            // read_flash_sequencers_status_init();
            // printf("sequencers.on_ff = %d",sequencers.on_ff);
            sequencers.open_timeing = 0;
            for(int i = 4, j = 0; i <= (3 + sequencers.relay_number); i++, j++)
            {
                sequencers.realy[j].open_time = uart2_data[i];
                if(sequencers.realy[j].open_time != 0)
                    sequencers.realy[j].open_on_off = DEVICE_ON;

                if(sequencers.realy[j].open_time > sequencers.open_timeing)
                    sequencers.open_timeing = sequencers.realy[j].open_time;
            }
            //应答返回
            fb_information[0] = 0xFE;
            fb_information[1] = 0X03;
            fb_information[2] = sequencers.addr;
            fb_information[3] = 0x04;
            fb_information[4] = 0x00;
            fb_information[5] = 0xFF;
            Uart2_Send_Tx(fb_information,6);  //返回
            save_sequencers_data_area3();
        }

        //2设置关机时序
        if(uart2_data[0] == 0xFE  && sequencers.addr == uart2_data[2] && uart2_data[3] == 0x06 )
        {
            // read_flash_sequencers_status_init();
            sequencers.close_timeing = 0;
            for(int i = 4, j = 0; i <= (3 + sequencers.relay_number); i++, j++)
            {
                sequencers.realy[j].close_time = uart2_data[i];
                sequencers.realy[j].clod_on_off = DEVICE_OFF;

                if(sequencers.realy[j].close_time > sequencers.close_timeing)
                    sequencers.close_timeing = sequencers.realy[j].close_time;
            }
            fb_information[0] = 0xFE;
            fb_information[1] = 0X03;
            fb_information[2] = sequencers.addr;
            fb_information[3] = 0x06;
            fb_information[4] = 0x00;
            fb_information[5] = 0xFF;
            Uart2_Send_Tx(fb_information,6);  //返回
            save_sequencers_data_area3();
        }


        //3查看开机时序
        if(uart2_data[0] == 0xFE && uart2_data[1] == 0x03 && sequencers.addr == uart2_data[2] && uart2_data[3] == 0x03 )
        {

            fb_information[0] = 0xFE;
            fb_information[1] = sequencers.relay_number + 2;
            fb_information[2] = sequencers.addr;
            fb_information[3] = 0x03;
            read_flash_sequencers_status_init();
            for(int i = 4, j = 0; i <= (3 + sequencers.relay_number); i++, j++)
            {
                fb_information[i] = sequencers.realy[j].open_time;
            }

            fb_information[(4 + sequencers.relay_number)] = 0xFF;
            printf_buf(fb_information,(5 + sequencers.relay_number));
            Uart2_Send_Tx(fb_information,(5 + sequencers.relay_number));  //返回
        }


        //4查看关机时序
        if(uart2_data[0] == 0xFE && uart2_data[1] == 0x03 && sequencers.addr == uart2_data[2] && uart2_data[3] == 0x05 )
        {
            fb_information[0] = 0xFE;
            fb_information[1] = sequencers.relay_number + 2;
            fb_information[2] = sequencers.addr;
            fb_information[3] = 0x05;
            read_flash_sequencers_status_init();
            for(int i = 4, j = 0; i <= (3 + sequencers.relay_number); i++, j++)
            {
                fb_information[i] = sequencers.realy[j].close_time;
            }

            fb_information[(4 + sequencers.relay_number)] = 0xFF;
            printf_buf(fb_information,(5 + sequencers.relay_number));
            Uart2_Send_Tx(fb_information,(5 + sequencers.relay_number));  //返回
        }


        //5查看设备运行状态
        if(uart2_data[0] == 0xFE  && uart2_data[1] == 0x03 && sequencers.addr ==  uart2_data[2] && uart2_data[3] == 0x08 && uart2_data[4] == 0x00 && uart2_data[5] == 0xFF )
        {
            printf("check status");
            fb_information[0] = 0xFE;
            fb_information[1] = sequencers.relay_number + 7;
            fb_information[2] = sequencers.addr;
            fb_information[3] = 0x08;
            fb_information[4] = 0x5A;  //z
            fb_information[5] = 0x44;  //d
            fb_information[6] = 0x00;  //正式版本号
            fb_information[7] = 0x02;   //送测版本号
            fb_information[8] = sequencers.on_ff;
            for(int i = 9, j = 0; i <= (8 + sequencers.relay_number); i++, j++)
            {
                fb_information[i] = temp_on_off[j];
            }

            fb_information[(9 + sequencers.relay_number)] = 0xFF;
            Uart2_Send_Tx(fb_information, (10 + sequencers.relay_number));
        }


        if(sequencers.on_ff == DEVICE_ON)
        {
            //6供电控制
            if(uart2_data[0] == 0xFE && uart2_data[1] == 0x03 && sequencers.addr == uart2_data[2] && uart2_data[3] != 0x00 &&  uart2_data[4] != 0x00)// && data_len == 6)
            {
                printf("control");
                u32 sw;
                if(uart2_data[3] == 0x01)
                {
                    printf("bihe");
                    temp_on_off[uart2_data[4] - 1] = DEVICE_ON;
                    sw = relay_table[uart2_data[4] -1];
                    relay_off_on(sw ,uart2_data[4] - 1);
                }
                if(uart2_data[3] == 0x02)
                {
                    printf("duankai");
                    temp_on_off[uart2_data[4] - 1] = DEVICE_OFF;
                    sw = relay_table[uart2_data[4] -1 ];
                    relay_off_on(sw,uart2_data[4] -1 );
                }
            }

            //7关联控制 即一台控制单元，由两个供电来同时控制，且为互斥
            if(uart2_data[0] == 0xFE && uart2_data[1] == 0x04 && sequencers.addr == uart2_data[2] && uart2_data[3] == 0x07)
            {
                if(uart2_data[4] != uart2_data[5])
                {
                    u32 swA,swB;
                    //打开供电口
                    temp_on_off[uart2_data[4] - 1] = DEVICE_OFF;
                    swA = relay_table[uart2_data[4] - 1] ;
                    relay_off_on(swA  ,uart2_data[4] - 1);
                    //关闭供电口
                    temp_on_off[uart2_data[5] - 1] = DEVICE_ON;
                    swB = relay_table[uart2_data[5] - 1];
                    relay_off_on(swB ,uart2_data[5] - 1);

                    fd_relay_state();
                }

            }

        }

        //地址不是本地设备，发送到级联设备
        if(uart2_data[2] != sequencers.addr && uart2_data[2] != 0)
        {
            Uart1_Send_Tx(uart2_data,data_len);
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
    sequencers.addr = 1; //0：作用是：地址需要设置了才能用
    sequencers.on_ff = DEVICE_OFF;
    sequencers.relay_number = RELAYS_MAX;
    sequencers.timeing_flag = 1;
    sequencers.open_timeing = 0; //默认设备开关机时序不计时

    //默认开机时逐个亮
    for(open_set_cnt = 0; open_set_cnt < RELAYS_MAX; open_set_cnt++)
    {
        sequencers.realy[open_set_cnt].open_time = open_set_cnt + 1;
        sequencers.realy[open_set_cnt].open_on_off = DEVICE_ON;


        sequencers.realy[open_set_cnt].countdown_open_time.year = 2023;
        sequencers.realy[open_set_cnt].countdown_open_time.month = 12;
        sequencers.realy[open_set_cnt].countdown_open_time.day = 12;
        sequencers.realy[open_set_cnt].countdown_open_time.hour = 12;
        sequencers.realy[open_set_cnt].countdown_open_time.min = 12;
        sequencers.realy[open_set_cnt].countdown_open_time.sec = 12;


    }
    //默认关机时逐个灭
    for(close_set_cnt = 0; close_set_cnt < RELAYS_MAX; close_set_cnt++ )
    {

        sequencers.realy[close_set_cnt].close_time = i;
        sequencers.realy[close_set_cnt].clod_on_off = DEVICE_OFF;
        i--;


        sequencers.realy[close_set_cnt].countdown_close_time.year = 2023;
        sequencers.realy[close_set_cnt].countdown_close_time.month = 12;
        sequencers.realy[close_set_cnt].countdown_close_time.day = 12;
        sequencers.realy[close_set_cnt].countdown_close_time.hour = 12;
        sequencers.realy[close_set_cnt].countdown_close_time.min = 12;
        sequencers.realy[close_set_cnt].countdown_close_time.sec = 12;



    }







}

/**
 * @brief  APP指令控制继电器
 *
 * @param relay_led   AD按键 灯
 * @param le_state    继电器
 */
extern  ON_OFF_FLAG temp_on_off[16];  //继电器的开关

void relay_off_on(u32 relay_led,u8 relay_number)
{

    if(temp_on_off[relay_number] == DEVICE_ON)
    {
        gpio_direction_output(relay_led, 1); //开灯
        adkey_ctrl_lcd_relays_open(relay_number); // lcd点亮对应的通道
    }

    else
    {
        gpio_direction_output(relay_led, 0); //关灯
        adkey_ctrl_lcd_relays_close(relay_number); // lcd点亮对应的通道
    }

}


void fd_relay_state(void)
{
    u8 fb_temp[50];
    fb_temp[0] = 0xFE;
    fb_temp[1] = sequencers.relay_number + 7;
    fb_temp[2] = sequencers.addr;
    fb_temp[3] = 0x08;
    fb_temp[4] = 0x5A;  //z
    fb_temp[5] = 0x44;  //d
    fb_temp[6] = 0x00;  //正式版本号
    fb_temp[7] = 0x01;   //送测版本号
    fb_temp[8] = sequencers.on_ff;
    for(int i = 9, j = 0; i <= (8 + sequencers.relay_number); i++, j++)
    {
        fb_temp[i] = temp_on_off[j];
    }

    fb_temp[(9 + sequencers.relay_number)] = 0xFF;
    Uart2_Send_Tx(fb_temp, (10 + sequencers.relay_number));
}



/**
 * @brief AD按键控制继电器
 *
 */
void adkey_control(u32 relay_led,u8 relay_number)
{
    temp_on_off[relay_number]= ! temp_on_off[relay_number];
    if(temp_on_off[relay_number]== DEVICE_ON)
    {
        gpio_direction_output(relay_led, 1); //开灯
        adkey_ctrl_lcd_relays_open(relay_number); // lcd点亮对应的通道
    }
    else
    {
        gpio_direction_output(relay_led, 0); //关灯
        adkey_ctrl_lcd_relays_close(relay_number); // lcd点亮对应的通道
    }

}





u8 sw0_led_flag;  //作用：灯闪烁
u16 timer_id = 0;                       // 定时器ID
u16 timer_cnt = 0;
int temp_time= 0;
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
    static u8 delay_2s_cnt = 0;
    sequencers.timeing_flag = 0;

    if (timer_cnt >= sequencers.close_timeing)  //条件必须是有等于，可能关机时序时0秒
    {
        sequencers.timeing_flag = 1;

    }
    if(delay_2s_close_f)
    {


        sys_s_hi_timer_del(timer_id);   // 注销定时器  停止计时
        timer_id = 0;                   // 防止重复注册

    //关机，点亮三个mp3按键的灯
            gpio_direction_output(IO_PORTA_11,0);
            gpio_direction_output(IO_PORTC_03,0);
            gpio_direction_output(IO_PORTC_02,0);

            //关机 关闭 LCD屏的背光灯
            gpio_direction_output(IO_PORTA_07, 0);

            lcd1621_off();  //关闭lcd显示

            app_task_switch_to(APP_SLEEP_TASK);

    }


    temp_time++;
    if((temp_time %= 2) == 0)
    {
       need_handle_relays(DEVICE_OFF);
        timer_cnt++;
    }

    master_led_flashing(); //关机时序 总开门的闪烁

    if(sequencers.timeing_flag == 1) //表示关机时序执行完成
    {
        all_shutdowm();  //确保所有继电器的状态是关机状态

        gpio_direction_output(sw0_led, 0); //关闭总开关的指示灯
        sequencers.on_ff = DEVICE_OFF;
        app_task_put_key_msg(APP_CMD, 0);  //推送按键消息
        //  for(int i = 0; i < 16; i++)
        //     printf("sequencers.realy[%d].open_on_off = %d", i,sequencers.realy[i].open_on_off);


        //为了延时2s才关闭屏幕的
        delay_2s_cnt++;
        if(delay_2s_cnt == 4) //500*4 = 2000ms
        {
            delay_2s_cnt = 0;

            delay_2s_close_f = 1;
        }


    }

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
    sequencers.timeing_flag = 0;
    if (timer_cnt >= sequencers.open_timeing)//条件必须是有等于，可能开机时序时0秒
    {
        sequencers.timeing_flag = 1;
        sys_s_hi_timer_del(timer_id);   // 注销定时器  停止计时
        timer_id = 0;                   // 防止重复注册
    }

    temp_time++;
    if((temp_time %= 2) == 0)   //每一秒扫描所有继电器状态
    {
        timer_cnt++;
        need_handle_relays(DEVICE_ON);
    }

    master_led_flashing();  //总开关灯闪

    if(sequencers.timeing_flag == 1)
    {
        gpio_direction_output(sw0_led, 1); //开灯
        clean_dis(clrbit(SEG_T));  //开机完后，关闭音符
        sequencers.on_ff = DEVICE_ON;
        app_task_put_key_msg(APP_CMD, 0);  //推送按键消息
        printf("open machine ");
    }

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

    u8 next_data[7];
    printf("adkey_master_on_off");
    if(sequencers.on_ff == DEVICE_OFF )    // ---------------------- 开机
    {
        printf("adkey_master_on_off   open");
        //开机，点亮三个mp3按键的灯
        gpio_direction_output(IO_PORTA_11,1);
        gpio_direction_output(IO_PORTC_03,1);
        gpio_direction_output(IO_PORTC_02,1);

        //开机点亮LCD屏的背光灯
        gpio_direction_output(lcd_light, 1); //背光灯默认关

        //lcd屏幕显示轮廓
        lcd_open_frame();



        read_flash_sequencers_status_init();  //读取开机时序信息
        find_max_time(DEVICE_ON);
        open_timer_test();//开始时序

        //实现一键开机
        next_data[0] = 0xFE;
        next_data[1] = 0x03;
        next_data[2] = 0x00;
        next_data[3] = 0x02;
        next_data[4] = 0x01;  //开机
        next_data[5] = 0xFF;
        Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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
    else if(sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
    {
         printf("adkey_master_on_off   off");

            read_flash_sequencers_status_init();  //读取关机时序信息
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
            Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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
}

// ---------------------------------------------  控制面板的功能逻辑  ----------------------------------------------------

#include "lcd1621.h"
extern u8 lcd_now_state;
u8 time_unit = 0;
u8 sys_time_unit = 0;
extern u8 blink_f;
u8 chose_relays_num = 0;
//使用数组的想法是，将8个继电器的临时时间分别存，这样可以不混乱，任意按键退出设置模式后，8路继电器都能保存

u8 split_open_time[8][4] = {0};
u8 split_close_time[8][4] = {0};
extern u16 blink_cnt ;

void make_lock_screen(void)
{
    clean_dis(clrbit(SEG_X3));
    make_dis(SEG_X1);  //
    make_dis(SEG_X2);

}


void dis_lock_screen(void)
{
    clean_dis(clrbit(SEG_X1));
    make_dis(SEG_X2);  //
    make_dis(SEG_X3);


}


#define pre_tiem 20



extern u8 temp_year[4] ;
extern u8 temp_month[2];
extern u8 temp_day[2] ;
extern u8 temp_hour[2] ;
extern u8 temp_min[2] ;
extern u8 temp_sec[2] ;

u8 set_countdown_open_year[8][4] = {0};
u8 set_countdown_open_month[8][2] = {0};
u8 set_countdown_open_day[8][2] = {0};
u8 set_countdown_open_hour[8][2] = {0};
u8 set_countdown_open_min[8][2] = {0};
u8 set_countdown_open_sec[8][2] = {0};

u8 set_countdown_close_year[8][4] = {0};
u8 set_countdown_close_month[8][2] = {0};
u8 set_countdown_close_day[8][2] = {0};
u8 set_countdown_close_hour[8][2] = {0};
u8 set_countdown_close_min[8][2] = {0};
u8 set_countdown_close_sec[8][2] = {0};




u8 m_arry[8] = {0,0,0,0,0,0,0,0};
u8 show_e_f = 0;
/**
 * @brief AD按键控制16路继电器
 *
 * @param keyevent    AD按键消息
 */
void adkey_16way_on_off(int keyevent)
{
    switch (keyevent)
    {
        //继电器
                             //控灯                       向上位机反馈
        case KEY0_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw1_led,0);   fd_relay_state();} break;
        case KEY1_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw2_led,1);   fd_relay_state();} break;
        case KEY2_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw3_led,2);   fd_relay_state();} break;
        case KEY3_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw4_led,3);   fd_relay_state();} break;
        case KEY4_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw5_led,4);   fd_relay_state();} break;
        case KEY5_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw6_led,5);   fd_relay_state();} break;
        case KEY6_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw7_led,6);   fd_relay_state();} break;
        case KEY7_AD_CLICK: if(lcd_now_state == show_power) { adkey_control(sw8_led,7);   fd_relay_state();} break;

        //mp3的按键
// --------------------------------------------------------------- 上调
        case KEY8_AD_CLICK:
        //调开机时序
        if(lcd_now_state == open_dev_time)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;

            switch(time_unit)
            {
                case 0:
                split_open_time[chose_relays_num][0] = 0;
                break;
                case 1:
                    if(split_open_time[chose_relays_num][1] < 4)
                        split_open_time[chose_relays_num][1]++;
                break;
                case 2:
                    if(split_open_time[chose_relays_num][1] < 4)
                    {
                        if( split_open_time[chose_relays_num][2] < 9)
                        split_open_time[chose_relays_num][2]++;
                    }
                    else
                    {
                        split_open_time[chose_relays_num][2] = 1;
                    }
                break;
                case 3:

                    if(split_open_time[chose_relays_num][1] < 4)
                    {
                        if( split_open_time[chose_relays_num][3] < 9)
                        split_open_time[chose_relays_num][3]++;
                    }
                    else
                    {
                        split_open_time[chose_relays_num][3] = 5;
                    }

                break;

            }

        }
        //调关机时序
        else if(lcd_now_state == close_dev_time)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;

            switch(time_unit)
            {
                case 0:
                    split_close_time[chose_relays_num][0] = 0;
                break;
                case 1:
                    if(split_close_time[chose_relays_num][1] < 4)
                    split_close_time[chose_relays_num][1]++;
                break;
                case 2:
                    if(split_close_time[chose_relays_num][1] < 4)
                    {
                        if( split_close_time[chose_relays_num][2] < 9)
                        split_close_time[chose_relays_num][2]++;
                    }
                    else
                    {
                        split_close_time[chose_relays_num][2] = 1;
                    }
                break;
                case 3:

                    if(split_close_time[chose_relays_num][1] < 4)
                    {
                        if( split_close_time[chose_relays_num][3] < 9)
                        split_close_time[chose_relays_num][3]++;
                    }
                    else
                    {
                        split_close_time[chose_relays_num][3] = 5;
                    }

                break;

            }
        }
        //功率默模式
        else if(lcd_now_state == show_power)
        {
            // bt_key_music_prev();  // 上一曲
        }
        /* 调系统时间 */
        else if(lcd_now_state == set_sys_time)
        {

            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;
            switch(sys_time_unit)
            {
                u8 ti = 0;
                // 年份
                case 0:
                if(temp_year[0] < 9)
                {
                    temp_year[0]++;
                }

                break;
                case 1:
                if(temp_year[1] < 9)
                {
                    temp_year[1]++;
                }
                break;
                case 2:
                if(temp_year[2] < 9)
                {
                    temp_year[2]++;
                }
                break;
                case 3:
                if(temp_year[3] < 9)
                {
                    temp_year[3]++;
                }
                break;
                // 月份
                case 4:

                if(temp_month[0] < 1)
                {
                    temp_month[0]++;
                }
                break;
                case 5:
                if(temp_month[0] == 1)
                {
                    if(temp_month[1] < 2)
                    {
                        temp_month[1]++;
                    }
                }
                else
                {
                    if(temp_month[1] < 9)
                    {
                        temp_month[1]++;
                    }
                }

                break;
                // 日
                case 6:
                if(temp_day[0] < 3)
                {
                    temp_day[0]++;
                }
                break;
                case 7:
                if(temp_day[0] == 3)
                {
                    if(temp_day[1] < 1)
                    {
                        temp_day[1]++;
                    }
                }
                else
                {
                    if(temp_day[1] < 9)
                    {
                        temp_day[1]++;
                    }
                }

                break;
                // 时
                case 8:
                if(temp_hour[0] < 2)
                {
                    temp_hour[0]++;
                }
                break;
                case 9:

                if(temp_hour[0] == 2)
                {
                    if(temp_hour[1] < 3)
                    {
                        temp_hour[1]++;
                    }
                }
                else
                {
                    if(temp_hour[1] < 9)
                    {
                        temp_hour[1]++;
                    }
                }
                break;
                // 分
                case 10:
                if(temp_min[0] < 5)
                {
                    temp_min[0]++;
                }
                break;
                case 11:
                if(temp_min[1] < 9)
                {
                    temp_min[1]++;
                }
                break;
                // 秒
                case 12:
                if(temp_sec[0] < 5)
                {
                    temp_sec[0]++;
                }
                break;
                case 13:
                if(temp_sec[1] < 9)
                {
                    temp_sec[1]++;
                }
                break;
            }
        }
        /* 调定时开继电器  */
        else if(lcd_now_state == timing_relay_open)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;
            switch(time_unit)
            {
                u8 ti = 0;
                // 年份
                case 0:
                if(set_countdown_open_year[chose_relays_num][0] < 9)
                {
                    set_countdown_open_year[chose_relays_num][0]++;
                }

                break;
                case 1:
                if(set_countdown_open_year[chose_relays_num][1] < 9)
                {
                    set_countdown_open_year[chose_relays_num][1]++;
                }
                break;
                case 2:
                if(set_countdown_open_year[chose_relays_num][2] < 9)
                {
                    set_countdown_open_year[chose_relays_num][2]++;
                }
                break;
                case 3:
                if(set_countdown_open_year[chose_relays_num][3] < 9)
                {
                    set_countdown_open_year[chose_relays_num][3]++;
                }
                break;
                // 月份
                case 4:

                if(set_countdown_open_month[chose_relays_num][0] < 1)
                {
                    set_countdown_open_month[chose_relays_num][0]++;
                }
                break;
                case 5:
                if(set_countdown_open_month[chose_relays_num][0] == 1)
                {
                    if(set_countdown_open_month[chose_relays_num][1] < 2)
                    {
                        set_countdown_open_month[chose_relays_num][1]++;
                    }
                }
                else
                {
                    if(set_countdown_open_month[chose_relays_num][1] < 9)
                    {
                        set_countdown_open_month[chose_relays_num][1]++;
                    }
                }

                break;
                // 日
                case 6:
                if(set_countdown_open_day[chose_relays_num][0] < 3)
                {
                    set_countdown_open_day[chose_relays_num][0]++;
                }
                break;
                case 7:
                if(set_countdown_open_day[chose_relays_num][0] == 3)
                {
                    if(set_countdown_open_day[chose_relays_num][1] < 1)
                    {
                        set_countdown_open_day[chose_relays_num][1]++;
                    }
                }
                else
                {
                    if(set_countdown_open_day[chose_relays_num][1] < 9)
                    {
                        set_countdown_open_day[chose_relays_num][1]++;
                    }
                }

                break;
                // 时
                case 8:
                if(set_countdown_open_hour[chose_relays_num][0] < 2)
                {
                    set_countdown_open_hour[chose_relays_num][0]++;
                }
                break;
                case 9:

                if(set_countdown_open_hour[chose_relays_num][0] == 2)
                {
                    if(set_countdown_open_hour[chose_relays_num][1] < 3)
                    {
                        set_countdown_open_hour[chose_relays_num][1]++;
                    }
                }
                else
                {
                    if(set_countdown_open_hour[chose_relays_num][1] < 9)
                    {
                        set_countdown_open_hour[chose_relays_num][1]++;
                    }
                }
                break;
                // 分
                case 10:
                if(set_countdown_open_min[chose_relays_num][0] < 5)
                {
                    set_countdown_open_min[chose_relays_num][0]++;
                }
                break;
                case 11:
                if(set_countdown_open_min[chose_relays_num][1] < 9)
                {
                    set_countdown_open_min[chose_relays_num][1]++;
                }
                break;
                // 秒
                case 12:
                if(set_countdown_open_sec[chose_relays_num][0] < 5)
                {
                    set_countdown_open_sec[chose_relays_num][0]++;
                }
                break;
                case 13:
                if(set_countdown_open_sec[chose_relays_num][1] < 9)
                {
                    set_countdown_open_sec[chose_relays_num][1]++;
                }
                break;
            }
        }
        /* 调定时关继电器 */
        else if(lcd_now_state == timing_relay_close)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;
            switch(time_unit)
            {
                u8 ti = 0;
                // 年份
                case 0:
                if(set_countdown_close_year[chose_relays_num][0] < 9)
                {
                    set_countdown_close_year[chose_relays_num][0]++;
                }

                break;
                case 1:
                if(set_countdown_close_year[chose_relays_num][1] < 9)
                {
                    set_countdown_close_year[chose_relays_num][1]++;
                }
                break;
                case 2:
                if(set_countdown_close_year[chose_relays_num][2] < 9)
                {
                    set_countdown_close_year[chose_relays_num][2]++;
                }
                break;
                case 3:
                if(set_countdown_close_year[chose_relays_num][3] < 9)
                {
                    set_countdown_close_year[chose_relays_num][3]++;
                }
                break;
                // 月份
                case 4:

                if(set_countdown_close_month[chose_relays_num][0] < 1)
                {
                    set_countdown_close_month[chose_relays_num][0]++;
                }
                break;
                case 5:
                if(set_countdown_close_month[chose_relays_num][0] == 1)
                {
                    if(set_countdown_close_month[chose_relays_num][1] < 2)
                    {
                        set_countdown_close_month[chose_relays_num][1]++;
                    }
                }
                else
                {
                    if(set_countdown_close_month[chose_relays_num][1] < 9)
                    {
                        set_countdown_close_month[chose_relays_num][1]++;
                    }
                }

                break;
                // 日
                case 6:
                if(set_countdown_close_day[chose_relays_num][0] < 3)
                {
                    set_countdown_close_day[chose_relays_num][0]++;
                }
                break;
                case 7:
                if(set_countdown_close_day[chose_relays_num][0] == 3)
                {
                    if(set_countdown_close_day[chose_relays_num][1] < 1)
                    {
                        set_countdown_close_day[chose_relays_num][1]++;
                    }
                }
                else
                {
                    if(set_countdown_close_day[chose_relays_num][1] < 9)
                    {
                        set_countdown_close_day[chose_relays_num][1]++;
                    }
                }

                break;
                // 时
                case 8:
                if(set_countdown_close_hour[chose_relays_num][0] < 2)
                {
                    set_countdown_close_hour[chose_relays_num][0]++;
                }
                break;
                case 9:

                if(set_countdown_close_hour[chose_relays_num][0] == 2)
                {
                    if(set_countdown_close_hour[chose_relays_num][1] < 3)
                    {
                        set_countdown_close_hour[chose_relays_num][1]++;
                    }
                }
                else
                {
                    if(set_countdown_close_hour[chose_relays_num][1] < 9)
                    {
                        set_countdown_close_hour[chose_relays_num][1]++;
                    }
                }
                break;
                // 分
                case 10:
                if(set_countdown_close_min[chose_relays_num][0] < 5)
                {
                    set_countdown_close_min[chose_relays_num][0]++;
                }
                break;
                case 11:
                if(set_countdown_close_min[chose_relays_num][1] < 9)
                {
                    set_countdown_close_min[chose_relays_num][1]++;
                }
                break;
                // 秒
                case 12:
                if(set_countdown_close_sec[chose_relays_num][0] < 5)
                {
                    set_countdown_close_sec[chose_relays_num][0]++;
                }
                break;
                case 13:
                if(set_countdown_close_sec[chose_relays_num][1] < 9)
                {
                    set_countdown_close_sec[chose_relays_num][1]++;
                }
                break;
            }

        }

        break;

// -------------------------------------------------------------- 播放

        //切换时间的位
        case KEY9_AD_CLICK:

        if(lcd_now_state == show_power)
        {
            // bt_key_music_pp();  // 播放/暂停
        }
        // 切换开关机延时的时间位
        else if(lcd_now_state == open_dev_time || lcd_now_state == close_dev_time )
        {
            blink_f = 1; blink_cnt = pre_tiem; //为了操作显示，感觉上流畅
            time_unit++; if(time_unit == 4) time_unit = 0;
        }
        // 切换定时开关继电器的时间位
        else if(lcd_now_state == timing_relay_open || lcd_now_state == timing_relay_close)
        {
            blink_f = 1; blink_cnt = pre_tiem; //为了操作显示，感觉上流畅
            time_unit++;
            if(time_unit == 14 ) time_unit = 0;  //
            if(time_unit < 8)
            {
                clean_num(1);clean_num(2);clean_num(3); // 清数据
                if(show_e_f== 2)
                {
                    make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器

                }
                else if(show_e_f == 3)
                {
                    make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器

                }
            }
            if(time_unit == 8)
            {
                clean_num(2);clean_num(3); // 清数据
            }


            }
        // 切换系统假时钟的时间位
        else if(lcd_now_state == set_sys_time )
        {
            blink_f = 1; blink_cnt = pre_tiem; //为了操作显示，感觉上流畅
            sys_time_unit++;
            if(sys_time_unit == 14 ) sys_time_unit = 0;  //
            // 如果日期的十位是3，切换一位是马上显示0
            if(sys_time_unit == 7)
            {
                if(temp_day[0] > 2)
                {
                    temp_day[1] = 0;
                }
            }
        }

        break;

// ------------------------------------------------------------------  下调
        case KEY10_AD_CLICK:
        //调开机时序
        if(lcd_now_state == open_dev_time)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = 20;

            switch(time_unit)
            {
                case 0:
                split_open_time[chose_relays_num][0] = 0;
                break;
                case 1:
                if(split_open_time[chose_relays_num][1] > 0)
                    split_open_time[chose_relays_num][1]--;
                break;
                case 2:
                    if(split_open_time[chose_relays_num][2] > 0)
                        split_open_time[chose_relays_num][2]--;

                break;
                case 3:

                if(split_open_time[chose_relays_num][3] > 0)
                        split_open_time[chose_relays_num][3]--;
                break;

            }

        }
        //调关机时序
        else if(lcd_now_state == close_dev_time)
        {

            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = 10;

            switch(time_unit)
            {
                case 0:
                split_close_time[chose_relays_num][0] = 0;
                break;
                case 1:
                if(split_close_time[chose_relays_num][1] > 0)
                    split_close_time[chose_relays_num][1]--;
                break;
                case 2:
                    if(split_close_time[chose_relays_num][2] > 0)
                        split_close_time[chose_relays_num][2]--;

                break;
                case 3:

                if(split_close_time[chose_relays_num][3] > 0)
                        split_close_time[chose_relays_num][3]--;
                break;

            }


        }
        //功率模式
        else if(lcd_now_state == show_power)
        {
            // bt_key_music_next();  // 下一曲
        }
        /* 调系统时间 */
        else if(lcd_now_state == set_sys_time)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;
            switch(sys_time_unit)
            {
                u8 ti = 0;
                // 年份
                case 0:
                if(temp_year[0] > 0)
                {
                    temp_year[0]--;
                }

                break;
                case 1:
                if(temp_year[1] > 0)
                {
                    temp_year[1]--;
                }
                break;
                case 2:
                if(temp_year[2] > 0)
                {
                    temp_year[2]--;
                }
                break;
                case 3:
                if(temp_year[3] > 0)
                {
                    temp_year[3]--;
                }
                break;
                // 月份
                case 4:
                if(temp_month[0] > 0)
                {
                    temp_month[0]--;
                }
                break;
                case 5:
                if(temp_month[1] > 0)
                {
                    temp_month[1]--;
                }
                break;
                // 日
                case 6:
                if(temp_day[0] > 0)
                {
                    temp_day[0]--;
                }
                break;
                case 7:
                if(temp_day[1] > 0)
                {
                    temp_day[1]--;
                }
                break;
                // 时
                case 8:
                if(temp_hour[0] > 0)
                {
                    temp_hour[0]--;
                }
                break;
                case 9:


                if(temp_hour[1] > 0)
                {
                    temp_hour[1]--;
                }



                break;
                // 分
                case 10:
                if(temp_min[0] > 0)
                {
                    temp_min[0]--;
                }


                break;
                case 11:
                if(temp_min[1] > 0)
                {
                    temp_min[1]--;
                }
                break;
                // 秒
                case 12:
                if(temp_sec[0] > 0)
                {
                    temp_sec[0]--;
                }
                break;
                case 13:
                if(temp_sec[1] > 0)
                {
                    temp_sec[1]--;
                }
                break;

            }

        }
        /* 调定时开继电器  */
        else if(lcd_now_state == timing_relay_open)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;
            switch(time_unit)
            {
                u8 ti = 0;
                // 年份
                case 0:
                if(set_countdown_open_year[chose_relays_num][0] > 0)
                {
                    set_countdown_open_year[chose_relays_num][0]--;
                }

                break;
                case 1:
                if(set_countdown_open_year[chose_relays_num][1] > 0)
                {
                    set_countdown_open_year[chose_relays_num][1]--;
                }
                break;
                case 2:
                if(set_countdown_open_year[chose_relays_num][2] > 0)
                {
                    set_countdown_open_year[chose_relays_num][2]--;
                }
                break;
                case 3:
                if(set_countdown_open_year[chose_relays_num][3] > 0)
                {
                    set_countdown_open_year[chose_relays_num][3]--;
                }
                break;
                // 月份
                case 4:
                if(set_countdown_open_month[chose_relays_num][0] > 0)
                {
                    set_countdown_open_month[chose_relays_num][0]--;
                }
                break;
                case 5:
                if(set_countdown_open_month[chose_relays_num][1] > 0)
                {
                    set_countdown_open_month[chose_relays_num][1]--;
                }
                break;
                // 日
                case 6:
                if(set_countdown_open_day[chose_relays_num][0] > 0)
                {
                    set_countdown_open_day[chose_relays_num][0]--;
                }
                break;
                case 7:
                if(set_countdown_open_day[chose_relays_num][1] > 0)
                {
                    set_countdown_open_day[chose_relays_num][1]--;
                }
                break;
                // 时
                case 8:
                if(set_countdown_open_hour[chose_relays_num][0] > 0)
                {
                    set_countdown_open_hour[chose_relays_num][0]--;
                }
                break;
                case 9:


                if(set_countdown_open_hour[chose_relays_num][1] > 0)
                {
                    set_countdown_open_hour[chose_relays_num][1]--;
                }



                break;
                // 分
                case 10:
                if(set_countdown_open_min[chose_relays_num][0] > 0)
                {
                    set_countdown_open_min[chose_relays_num][0]--;
                }


                break;
                case 11:
                if(set_countdown_open_min[chose_relays_num][1] > 0)
                {
                    set_countdown_open_min[chose_relays_num][1]--;
                }
                break;
                // 秒
                case 12:
                if(set_countdown_open_sec[chose_relays_num][0] > 0)
                {
                    set_countdown_open_sec[chose_relays_num][0]--;
                }
                break;
                case 13:
                if(set_countdown_open_sec[chose_relays_num][1] > 0)
                {
                    set_countdown_open_sec[chose_relays_num][1]--;
                }
                break;

            }
        }
        /* 调定时关继电器 */
        else if(lcd_now_state == timing_relay_close)
        {
            blink_f = 0;//为了操作显示，感觉上流畅
            blink_cnt = pre_tiem;
            switch(time_unit)
            {
                u8 ti = 0;
                // 年份
                case 0:
                if(set_countdown_close_year[chose_relays_num][0] > 0)
                {
                    set_countdown_close_year[chose_relays_num][0]--;
                }

                break;
                case 1:
                if(set_countdown_close_year[chose_relays_num][1] > 0)
                {
                    set_countdown_close_year[chose_relays_num][1]--;
                }
                break;
                case 2:
                if(set_countdown_close_year[chose_relays_num][2] > 0)
                {
                    set_countdown_close_year[chose_relays_num][2]--;
                }
                break;
                case 3:
                if(set_countdown_close_year[chose_relays_num][3] > 0)
                {
                    set_countdown_close_year[chose_relays_num][3]--;
                }
                break;
                // 月份
                case 4:
                if(set_countdown_open_month[chose_relays_num][0] > 0)
                {
                    set_countdown_open_month[chose_relays_num][0]--;
                }
                break;
                case 5:
                if(set_countdown_close_month[chose_relays_num][1] > 0)
                {
                    set_countdown_close_month[chose_relays_num][1]--;
                }
                break;
                // 日
                case 6:
                if(set_countdown_close_day[chose_relays_num][0] > 0)
                {
                    set_countdown_close_day[chose_relays_num][0]--;
                }
                break;
                case 7:
                if(set_countdown_close_day[chose_relays_num][1] > 0)
                {
                    set_countdown_close_day[chose_relays_num][1]--;
                }
                break;
                // 时
                case 8:
                if(set_countdown_close_hour[chose_relays_num][0] > 0)
                {
                    set_countdown_close_hour[chose_relays_num][0]--;
                }
                break;
                case 9:


                if(set_countdown_close_hour[chose_relays_num][1] > 0)
                {
                    set_countdown_close_hour[chose_relays_num][1]--;
                }



                break;
                // 分
                case 10:
                if(set_countdown_close_min[chose_relays_num][0] > 0)
                {
                    set_countdown_close_min[chose_relays_num][0]--;
                }


                break;
                case 11:
                if(set_countdown_close_min[chose_relays_num][1] > 0)
                {
                    set_countdown_close_min[chose_relays_num][1]--;
                }
                break;
                // 秒
                case 12:
                if(set_countdown_close_sec[chose_relays_num][0] > 0)
                {
                    set_countdown_close_sec[chose_relays_num][0]--;
                }
                break;
                case 13:
                if(set_countdown_close_sec[chose_relays_num][1] > 0)
                {
                    set_countdown_close_sec[chose_relays_num][1]--;
                }
                break;

            }
        }



        break;

        default: break;

    }
}

//将开机时拆分成分秒的格式
void split_open_minute_second()
{
    u8 i = 0;
    // read_flash_sequencers_status_init();
    for(i = 0; i < 8; i++)
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
    for(i = 0; i < 8; i++)
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
    for(i = 0; i < 8; i++)
    {
        sequencers.realy[i].open_time = (split_open_time[i][0] *10 + split_open_time[i][1]) * 60 +  (split_open_time[i][2] *10 + split_open_time[i][3]);
    }
}


//将设置完成的8路关机时间，存在结构体中
void sum_close_minute_second(void)
{
     u8 i = 0;
    for(i = 0; i < 8; i++)
    {
        sequencers.realy[i].close_time = (split_close_time[i][0] *10 + split_close_time[i][1]) * 60 +  (split_close_time[i][2] *10 + split_close_time[i][3]);
    }
}






void read_relays_countdown_open_time(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
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
    for(i = 0; i < 8; i++)
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
    for(i = 0; i < 8; i++)
    {
        sequencers.realy[i].countdown_open_time.year = set_countdown_open_year[i][0] * 1000 + set_countdown_open_year[i][1] * 100 + set_countdown_open_year[i][2] * 10 + set_countdown_open_year[i][3];
        sequencers.realy[i].countdown_open_time.month = set_countdown_open_month[i][0] * 10 + set_countdown_open_month[i][1];
        sequencers.realy[i].countdown_open_time.day = set_countdown_open_day[i][0] * 10 + set_countdown_open_day[i][1];
        sequencers.realy[i].countdown_open_time.hour =  set_countdown_open_hour[i][0] * 10 +  set_countdown_open_hour[i][1] ;
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
    for(i = 0; i < 8; i++)
    {
        sequencers.realy[i].countdown_close_time.year =  set_countdown_close_year[i][0] * 1000 +  set_countdown_close_year[i][1] * 100 + set_countdown_close_year[i][2] * 10 + set_countdown_close_year[i][3];
        sequencers.realy[i].countdown_close_time.month = set_countdown_close_month[i][0] * 10 + set_countdown_close_month[i][1];
        sequencers.realy[i].countdown_close_time.day = set_countdown_close_day[i][0] * 10 + set_countdown_close_day[i][1];
        sequencers.realy[i].countdown_close_time.hour =  set_countdown_close_hour[i][0] * 10 +  set_countdown_close_hour[i][1];
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







u8 loc_screen_f = 0;
extern u16 update_cnt;

/**
 * @brief 长按某个继电器按键 进入设置开关机延时的模式
 *
 * @param keyevent  按键键值消息
 */
void adkey_16way_long(int keyevent)
{
    static u8 m = 0;

    switch (keyevent)
    {

//第一路
        case KEY0_AD_LONG:
        chose_relays_num = 0;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();

            //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显 ’ ”
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));  // 清 v w
            split_close_minute_second();

            //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;
            show_e_f = 0;
            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间

            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));  // 清 ” ‘
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;

//第二路
        case KEY1_AD_LONG:
        chose_relays_num = 1;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();

        //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();

            //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
        // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间

            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
//第三路
        case KEY2_AD_LONG:
        chose_relays_num = 2;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();


            //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();

                //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间


            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
//第四路
        case KEY3_AD_LONG:
        chose_relays_num = 3;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();

        //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();

            //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
         // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间

            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
//第五路
        case KEY4_AD_LONG:
        chose_relays_num = 4;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();


            //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();

            //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间


            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
//第六路
        case KEY5_AD_LONG:
        chose_relays_num = 5;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();


            //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();
                //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);
            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间

            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
//第七路
        case KEY6_AD_LONG:
        chose_relays_num = 6;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();


            //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);
            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();

                //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间


            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
//第八路
        case KEY7_AD_LONG:
        chose_relays_num = 7;
        clean_num(1);clean_num(2);clean_num(3);   //清
        clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏

        //设置开机时间模式
        if(m_arry[chose_relays_num] == 0)
        {
            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(0);  make_num(3,chose_relays_num+1);  // 显示P  对应的第几个继电器
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
            make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
            split_open_minute_second();


            //读取当前的开机时序数据
            make_num(4,split_open_time[chose_relays_num][0]);
            make_num(5,split_open_time[chose_relays_num][1]);
            make_num(6,split_open_time[chose_relays_num][2]);
            make_num(7,split_open_time[chose_relays_num][3]);

            lcd_now_state = open_dev_time;
            m_arry[chose_relays_num] = 1;
        }
        // 设置关机时间模式
        else if(m_arry[chose_relays_num] == 1)
        {
            time_unit = 0;
            make_alphabet(1);  make_num(3,chose_relays_num+1);
            make_dis(SEG_S3);make_dis(SEG_S4);
            clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6));
            split_close_minute_second();

                //读取当前的关机时序
            make_num(4,split_close_time[chose_relays_num][0]);
            make_num(5,split_close_time[chose_relays_num][1]);
            make_num(6,split_close_time[chose_relays_num][2]);
            make_num(7,split_close_time[chose_relays_num][3]);

            lcd_now_state = close_dev_time;
            m_arry[chose_relays_num] = 2;

        }
        // 设置继电器定时开
        else if(m_arry[chose_relays_num] == 2)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(2);  make_num(3,chose_relays_num+1);  // 显示e  对应的第几个继电器
            read_relays_countdown_open_time();
            lcd_now_state = timing_relay_open;
            m_arry[chose_relays_num] = 3;
            show_e_f = 2;
        }
         // 设置继电器定时关
        else if(m_arry[chose_relays_num] == 3)
        {

            time_unit = 0; //确保每次进入，都是从分开始
            make_alphabet(3);  make_num(3,chose_relays_num+1);  // 显示f  对应的第几个继电器
            read_relays_countdown_close_time();
            lcd_now_state = timing_relay_close;
            m_arry[chose_relays_num] = 4;
            show_e_f = 3;
        }
        //退出设置模式
        else if(m_arry[chose_relays_num] == 4)
        {
            time_unit = 0;

            sum_open_minute_second();    // 记录8路的开机时间
            sum_close_minute_second();  // 记录8路的关机时间

            write_relays_countdown_open_time();     // 记录8路定时开的时间
            write_relays_countdown_close_time();    // 记录8路定时关的时间

            clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4));
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            lcd_now_state = show_power;
            m_arry[chose_relays_num] = 0;
            save_sequencers_data_area3();
        }

        break;
        case KEY8_AD_LONG:   break;
        case KEY9_AD_LONG:   /* 锁屏  设置系统时间 */
        if(lcd_now_state == set_sys_time)
        {
            lcd_now_state = show_power;
            update_cnt = 950;
            make_dis(SEG_S5); // "V"
            make_dis(SEG_S6); // "W"
            input_sys_time();  // 录入系统时间
        }
        else
        {
            loc_screen_f = 1;
            make_lock_screen();
        }


        break;
        case KEY10_AD_LONG:  break;

        default: break;

    }
}

/**
 * @brief 循环，查找需要操作的继电器
 *
 * @param temp
 */
extern ON_OFF_FLAG temp_on_off[16];

void need_handle_relays(ON_OFF_FLAG temp)
{
    u32 sw;
    if(temp == DEVICE_ON )   //开机
    {
        for(u8 i = 0; i < sequencers.relay_number;i++)
        {
            // printf("sequencers.realy[%d].open_on_off = %d", i,sequencers.realy[i].open_on_off);
            if(sequencers.realy[i].open_time != 0)
            {
                if(sequencers.realy[i].open_time == timer_cnt && sequencers.realy[i].open_on_off == DEVICE_ON)
                {
                    temp_on_off[i]  = sequencers.realy[i].open_on_off;
                    sw = relay_table[i];
                    relay_off_on(sw,i);
                }
            }
        }
    }
    if(temp == DEVICE_OFF)   //关机
    {
        for(u8 i = 0; i < sequencers.relay_number;i++)
        {
            if(sequencers.realy[i].close_time != 0)
            {
                if(sequencers.realy[i].close_time == timer_cnt && sequencers.realy[i].clod_on_off == DEVICE_OFF)
                {
                    temp_on_off[i]  = sequencers.realy[i].clod_on_off;
                    sw = relay_table[i];
                    relay_off_on(sw,i);
                }
            }
         }

    }

}

void all_shutdowm(void)
{
    u32 sw;
    for(u8 i = 0; i < RELAYS_MAX;i++)
    {
         temp_on_off[i] = DEVICE_OFF;
         sw = relay_table[i];
         relay_off_on(sw,i);
    }

}



/**
 * @brief 总开关灯闪烁
 *
 */
void master_led_flashing(void)
{
    if(sw0_led_flag)
        gpio_direction_output(sw0_led, 1); //开灯
    else
        gpio_direction_output(sw0_led, 0); //关灯

    sw0_led_flag = !sw0_led_flag;

}


// -------------------------------- 红外遥控  ---------------------------
//bt模式
void irkey_16way_click(int keyevent)
{



    u8 next_data[7];
    switch(keyevent)
    {
        case KEY1_IR_CLICK:  //开机
            if(sequencers.on_ff == DEVICE_OFF )    // ---------------------- 开机
            {
                printf("adkey_master_on_off   open");
                //开机，点亮三个mp3按键的灯
                gpio_direction_output(IO_PORTA_11,1);
                gpio_direction_output(IO_PORTC_03,1);
                gpio_direction_output(IO_PORTC_02,1);

                //开机点亮LCD屏的背光灯
                gpio_direction_output(lcd_light, 1); //背光灯默认关

                //lcd屏幕显示轮廓
                lcd_open_frame();



                read_flash_sequencers_status_init();  //读取开机时序信息
                find_max_time(DEVICE_ON);
                open_timer_test();//开始时序

                //实现一键开机
                next_data[0] = 0xFE;
                next_data[1] = 0x03;
                next_data[2] = 0x00;
                next_data[3] = 0x02;
                next_data[4] = 0x01;  //开机
                next_data[5] = 0xFF;
                Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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
            else if(sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
            {
                printf("adkey_master_on_off   off");

                read_flash_sequencers_status_init();  //读取关机时序信息
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
                Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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
            bt_key_music_prev();
        break;
        case KEY9_IR_CLICK:  // 播放/暂停
            bt_key_music_pp();

        break;
        case KEY8_IR_CLICK:  //下一曲
            bt_key_music_next();
        break;

        case KEY13_IR_CLICK:   //继电器1
        adkey_control(sw1_led,0);   fd_relay_state();
        break;
        case KEY14_IR_CLICK:
        adkey_control(sw2_led,1);   fd_relay_state();
        break;
        case KEY15_IR_CLICK:
        adkey_control(sw3_led,2);   fd_relay_state();
        break;
        case KEY16_IR_CLICK:
        adkey_control(sw4_led,3);   fd_relay_state();
        break;
        case KEY17_IR_CLICK:
        adkey_control(sw5_led,4);   fd_relay_state();
        break;
        case KEY18_IR_CLICK:
        adkey_control(sw6_led,5);   fd_relay_state();
        break;
        case KEY19_IR_CLICK:
        adkey_control(sw7_led,6);   fd_relay_state();
        break;
        case KEY20_IR_CLICK:
        adkey_control(sw8_led,7);   fd_relay_state();
        break;




    }







}

//仅在sleep调用
void irket_on_off(int keyevent)
{
     u8 next_data[6];
            printf("adkey_master_on_off   open");
            //开机，点亮三个mp3按键的灯
            gpio_direction_output(IO_PORTA_11,1);
            gpio_direction_output(IO_PORTC_03,1);
            gpio_direction_output(IO_PORTC_02,1);

            //开机点亮LCD屏的背光灯
            gpio_direction_output(lcd_light, 1); //背光灯默认关

            //lcd屏幕显示轮廓
            lcd_open_frame();



            read_flash_sequencers_status_init();  //读取开机时序信息
            find_max_time(DEVICE_ON);
            open_timer_test();//开始时序

            //实现一键开机
            next_data[0] = 0xFE;
            next_data[1] = 0x03;
            next_data[2] = 0x00;
            next_data[3] = 0x02;
            next_data[4] = 0x01;  //开机
            next_data[5] = 0xFF;
            Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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



    u8 next_data[7];
    switch(keyevent)
    {
        case KEY1_IR_CLICK:  //开机
            if(sequencers.on_ff == DEVICE_OFF )    // ---------------------- 开机
            {
                printf("adkey_master_on_off   open");
                //开机，点亮三个mp3按键的灯
                gpio_direction_output(IO_PORTA_11,1);
                gpio_direction_output(IO_PORTC_03,1);
                gpio_direction_output(IO_PORTC_02,1);

                //开机点亮LCD屏的背光灯
                gpio_direction_output(lcd_light, 1); //背光灯默认关

                //lcd屏幕显示轮廓
                lcd_open_frame();



                read_flash_sequencers_status_init();  //读取开机时序信息
                find_max_time(DEVICE_ON);
                open_timer_test();//开始时序

                //实现一键开机
                next_data[0] = 0xFE;
                next_data[1] = 0x03;
                next_data[2] = 0x00;
                next_data[3] = 0x02;
                next_data[4] = 0x01;  //开机
                next_data[5] = 0xFF;
                Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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
            else if(sequencers.on_ff == DEVICE_ON)   // -------------------------- 关机
            {
                printf("adkey_master_on_off   off");

                read_flash_sequencers_status_init();  //读取关机时序信息
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
                Uart1_Send_Tx(next_data,6); //通过串口1发送给级联设备

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
        adkey_control(sw1_led,0);   fd_relay_state();
        break;
        case KEY14_IR_CLICK:
        adkey_control(sw2_led,1);   fd_relay_state();
        break;
        case KEY15_IR_CLICK:
        adkey_control(sw3_led,2);   fd_relay_state();
        break;
        case KEY16_IR_CLICK:
        adkey_control(sw4_led,3);   fd_relay_state();
        break;
        case KEY17_IR_CLICK:
        adkey_control(sw5_led,4);   fd_relay_state();
        break;
        case KEY18_IR_CLICK:
        adkey_control(sw6_led,5);   fd_relay_state();
        break;
        case KEY19_IR_CLICK:
        adkey_control(sw7_led,6);   fd_relay_state();
        break;
        case KEY20_IR_CLICK:
        adkey_control(sw8_led,7);   fd_relay_state();
        break;




    }

}








#if 0
/*
    [[  注意!!!  ]]
    * 如果当系统任务较少时使用本demo，需要将低功耗关闭（#define TCFG_LOWPOWER_LOWPOWER_SEL    0//SLEEP_EN ），否则任务被串口接收函数调用信号量pend时会导致cpu休眠，串口中断和DMA接收将遗漏数据或数据不正确
*/

#define UART_DEV_USAGE_TEST_SEL         2       //uart_dev.c api接口使用方法选择
//  选择1  串口中断回调函数推送事件，由事件响应函数接收串口数据
//  选择2  由task接收串口数据

#define UART_DEV_TEST_MULTI_BYTE        1       //uart_dev.c 读写多个字节api / 读写1个字节api 选择

#define UART_DEV_FLOW_CTRL				0

static u8 uart_cbuf[512] __attribute__((aligned(4)));
static u8 uart_rxbuf[512] __attribute__((aligned(4)));

static void my_put_u8hex(u8 dat)
{
    u8 tmp;
    tmp = dat / 16;
    if (tmp < 10) {
        putchar(tmp + '0');
    } else {
        putchar(tmp - 10 + 'A');
    }
    tmp = dat % 16;
    if (tmp < 10) {
        putchar(tmp + '0');
    } else {
        putchar(tmp - 10 + 'A');
    }
    putchar(0x20);
}

//设备事件响应demo
static void uart_event_handler(struct sys_event *e)
{
    const uart_bus_t *uart_bus;
    u32 uart_rxcnt = 0;

    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            /* printf("uart event: DEVICE_EVENT_FROM_UART_RX_OVERFLOW\n"); */
            uart_bus = (const uart_bus_t *)e->u.dev.value;
            uart_rxcnt = uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
            if (uart_rxcnt) {
                printf("get_buffer:\n");
                for (int i = 0; i < uart_rxcnt; i++) {
                    my_put_u8hex(uart_rxbuf[i]);
                    if (i % 16 == 15) {
                        putchar('\n');
                    }
                }
                if (uart_rxcnt % 16) {
                    putchar('\n');
                }
#if (!UART_DEV_FLOW_CTRL)
                uart_bus->write(uart_rxbuf, uart_rxcnt);
#endif
            }
            printf("uart out\n");
        }
    }
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            /* printf("uart event:DEVICE_EVENT_FROM_UART_RX_OUTTIME\n"); */
            uart_bus = (const uart_bus_t *)e->u.dev.value;
            uart_rxcnt = uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
            if (uart_rxcnt) {
                printf("get_buffer:\n");
                for (int i = 0; i < uart_rxcnt; i++) {
                    my_put_u8hex(uart_rxbuf[i]);
                    if (i % 16 == 15) {
                        putchar('\n');
                    }
                }
                if (uart_rxcnt % 16) {
                    putchar('\n');
                }
#if (!UART_DEV_FLOW_CTRL)
                uart_bus->write(uart_rxbuf, uart_rxcnt);
#endif
            }
            printf("uart out\n");
        }
    }
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_event_handler, 0);

static void uart_u_task(void *arg)
{
    const uart_bus_t *uart_bus = arg;
    int ret;
    u32 uart_rxcnt = 0;

    printf("uart_u_task start\n");
    while (1) {
#if !UART_DEV_TEST_MULTI_BYTE
        //uart_bus->getbyte()在尚未收到串口数据时会pend信号量，挂起task，直到UART_RX_PND或UART_RX_OT_PND中断发生，post信号量，唤醒task
        ret = uart_bus->getbyte(&uart_rxbuf[0], 0);
        if (ret) {
            uart_rxcnt = 1;
            printf("get_byte: %02x\n", uart_rxbuf[0]);
            uart_bus->putbyte(uart_rxbuf[0]);
        }
#else
        //uart_bus->read()在尚未收到串口数据时会pend信号量，挂起task，直到UART_RX_PND或UART_RX_OT_PND中断发生，post信号量，唤醒task
        uart_rxcnt = uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
        if (uart_rxcnt) {
            printf("get_buffer:\n");
            for (int i = 0; i < uart_rxcnt; i++) {
                my_put_u8hex(uart_rxbuf[i]);
                if (i % 16 == 15) {
                    putchar('\n');
                }
            }
            if (uart_rxcnt % 16) {
                putchar('\n');
            }
#if (!UART_DEV_FLOW_CTRL)
            uart_bus->write(uart_rxbuf, uart_rxcnt);
#endif
        }
#endif
    }
}

static void uart_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("uart_rx_isr\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart_rx_ot_isr\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void *)DEVICE_EVENT_FROM_UART_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
}

static void uart_flow_ctrl_task(void *arg)
{
    const uart_bus_t *uart_bus = arg;
	while (1) {
		uart_bus->write("flow control test ", sizeof("flow control test "));
		os_time_dly(100);
	}
}

void uart_dev_test_main()
{
    const uart_bus_t *uart_bus;
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = IO_PORTA_01;
    u_arg.rx_pin = IO_PORTA_02;
    u_arg.rx_cbuf = uart_cbuf;
    u_arg.rx_cbuf_size = 512;
    u_arg.frame_length = 32;
    u_arg.rx_timeout = 100;
    u_arg.isr_cbfun = uart_isr_hook;
    u_arg.baud = 9600;
    u_arg.is_9bit = 0;
#if UART_DEV_FLOW_CTRL
    u_arg.tx_pin = IO_PORTA_00;
    u_arg.rx_pin = IO_PORTA_01;
    u_arg.baud = 1000000;
	extern void flow_ctl_hw_init(void);
	flow_ctl_hw_init();
#endif
    uart_bus = uart_dev_open(&u_arg);
    if (uart_bus != NULL) {
        printf("uart_dev_open() success\n");
#if (UART_DEV_USAGE_TEST_SEL == 2)
        os_task_create(uart_u_task, (void *)uart_bus, 31, 512, 0, "uart_u_task");
#endif
#if UART_DEV_FLOW_CTRL
		os_task_create(uart_flow_ctrl_task, (void *)uart_bus, 31, 128, 0, "flow_ctrl");
#endif
    }
}

#if UART_DEV_FLOW_CTRL
void uart_change_rts_state(void)
{
	static u8 rts_state = 1;
	extern void change_rts_state(u8 state);
	change_rts_state(rts_state);
	rts_state = !rts_state;
}
#endif

#endif
