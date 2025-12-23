#include "user_uart2_driver.h"
#include "event.h"
#include "instruction_handle.h" // 包含指令接收的相关接口

#define UART_RXBUF_SIZE             128
#define UART_CBUF_SIZE              128
#define UART_TIMEROUT               50 


#define UART_BAUDRATE               9600

//串口2 
#define UART2_TX_PORT               IO_PORTA_09
#define UART2_RX_PORT               IO_PORTA_10

// 定义系统事件类型，不能与 "event.h" 中定义的类型相同 
#define DEVICE_EVENT_FROM_UART2_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('2' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART2_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('3' << 8) | '\0')

static volatile u8 uart2_cbuf[UART_RXBUF_SIZE] __attribute__((aligned(4)));
static volatile u8 uart2_rxbuf[UART_RXBUF_SIZE] __attribute__((aligned(4)));
const uart_bus_t* uart2_bus = NULL;
static volatile uint32_t uart2_rcv_len = 0;
// static volatile uint8_t uart2_rcv_flag = 1;

static void Uart2_isr_hook(void* arg, u32 status); // 声明

u8 Uart2_Init(void)
{
    struct uart_platform_data_t u_arg = { 0 };
    u_arg.tx_pin = UART2_TX_PORT;
    u_arg.rx_pin = UART2_RX_PORT;
    u_arg.rx_cbuf = uart2_cbuf;
    u_arg.rx_cbuf_size = UART_CBUF_SIZE;
    u_arg.frame_length = UART_RXBUF_SIZE;
    u_arg.rx_timeout = UART_TIMEROUT;
    u_arg.isr_cbfun = Uart2_isr_hook;
    u_arg.is_9bit = 0;
    u_arg.baud = UART_BAUDRATE;

    // printf("Uart2_Init");

    uart2_bus = uart_dev_open(&u_arg);
    if (uart2_bus != NULL)
    {
        gpio_set_pull_up(u_arg.rx_pin, 1);
        return 1;
    }
    return 0;
}

void Uart2_Send_Tx(u8* txBuf, u8 len)
{
    if (uart2_bus)
    {
        uart2_bus->write(txBuf, len);
    }
}



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
        // 接收缓冲区满
        printf("uart 2 rx buff overflow \n");

        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART2_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }

    if (status == UT_RX_OT) {
        // 接收超时
        printf("uart 2 rx timeout \n");

        e.type = SYS_DEVICE_EVENT;
        e.arg = (void*)DEVICE_EVENT_FROM_UART2_RX_OUTTIME;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }
}

/**
 * @brief 串口2 设备事件处理
 *
 * @param e 系统事件->串口中断事件
 */
void uart2_event_handler(struct sys_event* e)
{
    //串口2  耀祥时序器设备与PC端通信 （也可能是下一级设备与上一级设备通信）

    // 串口2接收缓冲区溢出
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART2_RX_OVERFLOW) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: uart2 rx overflow\n");
            uart2_bus = (const uart_bus_t*)e->u.dev.value;
            uart2_rcv_len = uart2_bus->read(uart2_rxbuf, sizeof(uart2_rxbuf), 0);
            // printf_buf(uart2_rxbuf, uart2_rcv_len);
            // printf("\n");

            if (uart2_rcv_len) {
                for (u32 i = 0; i < uart2_rcv_len; i++) {
                    instruction_buffer_put(uart2_rxbuf[i]);
                }
            }

        }
    }

    // 串口2接收超时
    if ((u32)e->arg == DEVICE_EVENT_FROM_UART2_RX_OUTTIME) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event:uart2 rx timeout\n");
            uart2_bus = (const uart_bus_t*)e->u.dev.value;
            uart2_rcv_len = uart2_bus->read(uart2_rxbuf, sizeof(uart2_rxbuf), 0);
            // printf_buf(uart2_rxbuf, uart2_rcv_len);
            // printf("\n");

            if (uart2_rcv_len) {
                for (u32 i = 0; i < uart2_rcv_len; i++) {
                    instruction_buffer_put(uart2_rxbuf[i]);
                }
            }

        }
    }
}

SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart2_event_handler, 0);   //线程



