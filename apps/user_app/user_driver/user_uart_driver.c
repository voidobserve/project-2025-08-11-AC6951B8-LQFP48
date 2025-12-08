#include "user_uart_driver.h"
#include "../../../include_lib/system/event.h"

#define UART_DEV_USAGE_TEST_SEL         1       //uart_dev.c api接口使用方法选择
//  选择1  串口中断回调函数推送事件，由事件响应函数接收串口数据
//  选择2  由task接收串口数据


#define UART_RXBUF_SIZE             512
#define UART_CBUF_SIZE              512
#define UART_TIMEROUT               50 

#define UART_EN_NUM                 2

#define UART_BAUDRATE               115200

//串口0
#define UART0_TX_PORT               IO_PORTB_04
#define UART0_RX_PORT               IO_PORTB_06

#define DEVICE_EVENT_FROM_UART0_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('4' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART0_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('5' << 8) | '\0')

static u8 uart0_cbuf[512] __attribute__((aligned(4)));
static u8 uart0_rxbuf[512] __attribute__((aligned(4)));
const uart_bus_t* uart0_bus = NULL;
static volatile uint32_t uart0_rcv_len = 0;
static volatile uint8_t uart0_rcv_flag = 1;

//end



//串口2
#if UART_EN_NUM == 2
#define UART2_TX_PORT               IO_PORTA_09
#define UART2_RX_PORT               IO_PORTA_10

#define DEVICE_EVENT_FROM_UART2_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('2' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART2_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('3' << 8) | '\0')

static u8 uart2_cbuf[512] __attribute__((aligned(4)));
static u8 uart2_rxbuf[512] __attribute__((aligned(4)));
const uart_bus_t* uart2_bus = NULL;
static volatile uint32_t uart2_rcv_len = 0;
static volatile uint8_t uart2_rcv_flag = 1;
#endif
//end

static void Uart0_isr_hook(void* arg, u32 status); // 声明


void Uart2_Send_Tx(u8* txBuf, u8 len)
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
 * @brief 耀祥时序器串口0初始化    接功率计
 *
 * @return u8
 */
u8 Uart0_Init(void)
{
    struct uart_platform_data_t u_arg = { 0 };
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
 * @brief 耀祥时序器串口0中断函数
 *
 * @param arg
 * @param status
 */
static void Uart0_isr_hook(void* arg, u32 status)
{
    const uart_bus_t* ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("Uart0_isr_hook\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART0_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart0_rx_ot_isr   2\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART0_RX_OUTTIME;
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
static void Uart2_isr_hook(void* arg, u32 status)
{
    const uart_bus_t* ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("Uart2_isr_hook\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART2_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart2_rx_ot_isr   2\n");
#if (UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART2_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
}
#endif



u8 Uart2_Init(void)
{
#if UART_EN_NUM == 2
    struct uart_platform_data_t u_arg = { 0 };
    u_arg.tx_pin = UART2_TX_PORT;
    u_arg.rx_pin = UART2_RX_PORT;
    u_arg.rx_cbuf = uart2_cbuf;
    u_arg.rx_cbuf_size = UART_CBUF_SIZE;
    u_arg.frame_length = UART_RXBUF_SIZE;
    u_arg.rx_timeout = UART_TIMEROUT;
    u_arg.isr_cbfun = Uart2_isr_hook;
    u_arg.baud = UART_BAUDRATE;
    u_arg.is_9bit = 0;

    // printf("Uart2_Init");

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
 * @brief  串口消息处理 放在系统线程
 *
 * @param e
 */
void uart_event_handler(struct sys_event* e)
{
    const uart_bus_t* uart1_bus;
    u32 uart_rxcnt = 0;

    //串口0 耀祥时序器的功率计
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART0_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: DEVICE_EVENT_FROM_UART0_RX_OVERFLOW\n");
            // uart0_bus = (const uart_bus_t*)e->u.dev.value;
            // uart0_rcv_len = uart0_bus->read(uart0_rxbuf, sizeof(uart0_rxbuf), 0);

            // printf_buf(uart0_rxbuf, uart0_rcv_len);

            // parse_uart0_data(uart0_rxbuf, uart0_rcv_len);  // 解析数据 
            // //    Uart1_Send_Tx(uart0_rxbuf,uart0_rcv_len);

            // if (uart0_rcv_len) {
            //     uart0_rcv_flag = 1;

            //     Uart0_Rx_Deal(uart0_rxbuf, uart0_rcv_len); // 清空缓冲区中的数据
        }

    }

    if ((u32)e->arg == DEVICE_EVENT_FROM_UART0_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event:DEVICE_EVENT_FROM_UART0_RX_OUTTIME\n");
            // uart2_bus = (const uart_bus_t*)e->u.dev.value;
            // uart0_rcv_len = uart2_bus->read(uart0_rxbuf, sizeof(uart0_rxbuf), 0);

            // printf_buf(uart0_rxbuf, uart0_rcv_len);

            // //  Uart1_Send_Tx(uart0_rxbuf,uart0_rcv_len);

            // parse_uart0_data(uart0_rxbuf, uart0_rcv_len);

            // if (uart0_rcv_len) {
            //     uart0_rcv_flag = 1;

            //     Uart0_Rx_Deal(uart0_rxbuf, uart0_rcv_len);
            // }

        }
    }
    //串口0 end

  

    //串口2  耀祥时序器设备与PC端通信
#if UART_EN_NUM == 2

    // 串口2接收缓冲区溢出
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART2_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: DEVICE_EVENT_FROM_UART_RX_OVERFLOW\n");
            uart2_bus = (const uart_bus_t*)e->u.dev.value;
            uart2_rcv_len = uart2_bus->read(uart2_rxbuf, sizeof(uart2_rxbuf), 0);
            printf_buf(uart2_rxbuf, uart2_rcv_len);
            if (uart2_rcv_len) {
                uart2_rcv_flag = 1;
                // uart2_bus->write(uart2_rxbuf, uart2_rcv_len);
                // NextMCU Msg
                // Uart2_Rx_Deal(uart2_rxbuf, uart2_rcv_len);
            }

        }
    }

    // 串口2接收超时
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART2_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event:DEVICE_EVENT_FROM_UART_RX_OUTTIME\n");
            uart2_bus = (const uart_bus_t*)e->u.dev.value;
            uart2_rcv_len = uart2_bus->read(uart2_rxbuf, sizeof(uart2_rxbuf), 0);
            printf_buf(uart2_rxbuf, uart2_rcv_len);
            parse_uart2_data(uart2_rxbuf, uart2_rcv_len);

            if (uart2_rcv_len) {
                uart2_rcv_flag = 1;
                // uart2_bus->write(uart2_rxbuf, uart2_rcv_len);
                // NextMCU Msg
                // Uart2_Rx_Deal(uart2_rxbuf, uart2_rcv_len);
            }

        }
    }

    // app_task_put_usr_msg(APP_MSG_SYS_EVENT,0);

#endif

//串口2 end
}


SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_event_handler, 0);   //线程

