#include "uart_handle.h" 

// USER_TO_DO  可能要改成应用层的命名，不用uart，改成 instruct ，文件改成 instruct_handle.c

#define UART1_RX_BUFFER_LEN ((u8) 50) // 环形缓冲区长度

// 定义环形缓冲区结构体类型：
typedef struct
{
    u8 buffer[UART1_RX_BUFFER_LEN];
    u8 head;
    u8 tail;
    u8 count;
} uart1_rxbuffer_info_t;

static volatile uart1_rxbuffer_info_t uart1_rxbuffer_info;
 

u8 uart1_rxbuffer_get_count(void)
{
    return uart1_rxbuffer_info.count;
}

u8 uart1_rxbuffer_get(void)
{
    u8 rxbyte;

    if (0 == uart1_rxbuffer_info.count)
    {
        // 缓冲区空
        return 0;
    }

    // 先偏移索引，再取出数据
    uart1_rxbuffer_info.tail = (uart1_rxbuffer_info.tail + 1) % UART1_RX_BUFFER_LEN;
    rxbyte = uart1_rxbuffer_info.buffer[uart1_rxbuffer_info.tail];

    uart1_rxbuffer_info.count--;

    return rxbyte;
}

void uart1_rxbuffer_put(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    uart1_rxbuffer_info.head = (uart1_rxbuffer_info.head + 1) % UART1_RX_BUFFER_LEN;
    uart1_rxbuffer_info.buffer[uart1_rxbuffer_info.head] = byte;

    uart1_rxbuffer_info.count++;

    if (uart1_rxbuffer_info.count > UART1_RX_BUFFER_LEN)
    {
        uart1_rxbuffer_info.count = UART1_RX_BUFFER_LEN;
    }
}
