#include "user_uart1_driver.h"
#include "../../../include_lib/system/event.h"

#define UART1_RXBUF_SIZE             128 // 接收缓冲区大小
#define UART1_RXCBUF_SIZE            128 // 接收溢出产生中断的大小
#define UART1_RX_TIMEROUT             50 // 接收超时时间

#define UART1_BAUDRATE               9600


//串口1
#define UART1_TX_PORT               IO_PORTC_00
#define UART1_RX_PORT               IO_PORTC_01


static volatile u8 uart1_cbuf[UART1_RXBUF_SIZE] __attribute__((aligned(4)));
static volatile u8 uart1_rxbuf[UART1_RXBUF_SIZE] __attribute__((aligned(4)));
const uart_bus_t* uart1_bus = NULL;
static volatile uint32_t uart1_rcv_len = 0;


static void Uart1_isr_hook(void* arg, u32 status); // 函数声明

/**
 * @brief 串口1初始化
 *
 * @return u8
 */
u8 Uart1_Init(void)
{
    struct uart_platform_data_t u_arg = { 0 };
    u_arg.tx_pin = UART1_TX_PORT;
    u_arg.rx_pin = UART1_RX_PORT;
    u_arg.rx_cbuf = uart1_cbuf;
    u_arg.rx_cbuf_size = UART1_RXCBUF_SIZE;
    u_arg.frame_length = UART1_RXBUF_SIZE;
    u_arg.rx_timeout = UART1_RX_TIMEROUT;
    u_arg.isr_cbfun = Uart1_isr_hook;
    u_arg.is_9bit = 0;
    u_arg.baud = UART1_BAUDRATE;

    uart1_bus = uart_dev_open(&u_arg); // 初始化 uart 模块
    if (uart1_bus != NULL)
    {
        gpio_set_pull_up(u_arg.rx_pin, 1);
        return 1;
    }

    return 0;
}

///Uart TX 写入
void Uart1_Send_Tx(u8* txBuf, u8 len)
{
    if (uart1_bus)
    {
        uart1_bus->write(txBuf, len);
    }
}


/**
 * @brief 串口1回调函数 （相当于发送和接收中断）
 *
 * @param arg
 * @param status
 */
static void Uart1_isr_hook(void* arg, u32 status)
{
    const uart_bus_t* ubus = arg;
    struct sys_event e;
    // printf("Uart1_isr_hook\n");
    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("uart 1 rx buff overflow\n"); // 接收缓冲区满

        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART_RX_OVERFLOW; // 使用了默认的系统事件，uart rx overflow
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }

    if (status == UT_RX_OT) {
        printf("uart 1 rx timeout\n"); // 接收超时

        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART_RX_OUTTIME; // 使用了默认的系统事件，uart rx outtime
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }
}


/**
 * @brief 串口1 设备事件处理
 *
 * @param e 系统事件->串口中断事件
 */
void uart1_event_handler(struct sys_event* e)
{
    //串口1  发向耀祥时序器的级联设备
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            // printf("uart event: DEVICE_EVENT_FROM_UART_RX_OVERFLOW\n");
            uart1_bus = (const uart_bus_t*)e->u.dev.value;
            uart1_rcv_len = uart1_bus->read(uart1_rxbuf, sizeof(uart1_rxbuf), 0);   //接受到字符串实际长度
            // printf_buf(uart1_rxbuf, uart1_rcv_len);

            if (uart1_rcv_len) {
                // 处理接收到的数据
                instruction_feedback_buffer(uart1_rxbuf, uart1_rcv_len);

                // 往指令处理数组中存放数据
                // for (u32 i = 0; i < uart1_rcv_len; i++) {
                //     // printf("uart1_rxbuf[%u] = 0x%02x\n", (u16)i, uart1_rxbuf[i]);
                //     // instruction_buffer_put(uart1_rxbuf[i]);
                // }
            }
        }
    }

    if ((u32)e->arg == DEVICE_EVENT_FROM_UART_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            // printf("uart event:DEVICE_EVENT_FROM_UART_RX_OUTTIME\n");
            uart1_bus = (const uart_bus_t*)e->u.dev.value;
            uart1_rcv_len = uart1_bus->read(uart1_rxbuf, sizeof(uart1_rxbuf), 0);
            // printf_buf(uart1_rxbuf, uart1_rcv_len);

            //接受完数据就清
            if (uart1_rcv_len) {
                // 往指令处理数组中存放数据
                instruction_feedback_buffer(uart1_rxbuf, uart1_rcv_len);

                // 往指令处理数组中存放数据
                // for (u32 i = 0; i < uart1_rcv_len; i++) {
                //     // printf("uart1_rxbuf[%u] = 0x%02x\n", (u16)i, uart1_rxbuf[i]);
                //     // instruction_buffer_put(uart1_rxbuf[i]);
                // }
            }
        }
    }
    //串口1 end
}
// 将该函数加入SYS_DEVICE_EVENT队列中, 当队列中有新的推送消息时, 会进入这里进行判断
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart1_event_handler, 0);   // 

