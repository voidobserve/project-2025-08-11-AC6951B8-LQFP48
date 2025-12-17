#include "instruction_handle.h" 
#include "user_config.h"
#include "instruction_handler_func.h"

#define INSTRUCTION_BUFFER_LEN ((u16) 128) // 环形缓冲区长度

// 定义环形缓冲区结构体类型：
typedef struct
{
    u8 buffer[INSTRUCTION_BUFFER_LEN];
    u8 head;
    u8 tail;
    u16 count;
} instruction_buffer_info_t;

// 定义环形缓冲区结构体变量：（由串口更新，由指令扫描函数取出数据）
static volatile instruction_buffer_info_t instruction_buffer_info;

static volatile u8 cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
static volatile u8 cur_recv_instruction_index = 0; // 存放当前正在接收的数据的索引
static volatile u8 dest_recv_instruction_len = 0; // 记录当前要接收的数据指令长度

// 存放当前接收到的一条指令：
volatile u8 recv_instruction_buff[20] = { 0 };

typedef void (*instruction_handler_t)(void);

// 定义 指令与对应的处理函数查找表
// rf24_key_handle_func_buff
// const instruction_handler_t instruction_handler_func_buff[10] =
// {
//    [INSTRUCTION_TYPE_DEVICE_ON_OFF] = handle_device_on_off,
//     // handle_relay_on_off,
//     // handle_relay_deactive_time,
//     // handle_relay_active_time,
// }; 

u16 instruction_buffer_get_count(void)
{
    return instruction_buffer_info.count;
}


u8 instruction_buffer_get(void)
{
    u8 rxbyte;

    if (0 == instruction_buffer_info.count)
    {
        // 缓冲区空
        return 0;
    }

    // 先偏移索引，再取出数据
    instruction_buffer_info.tail = (instruction_buffer_info.tail + 1) % INSTRUCTION_BUFFER_LEN;
    rxbyte = instruction_buffer_info.buffer[instruction_buffer_info.tail];

    instruction_buffer_info.count--;

    return rxbyte;
}

/**
 * @brief 往缓冲区中更新数据，由串口接收来调用更新
 *
 * @param byte
 */
void instruction_buffer_put(u8 byte)
{
    // 目前的逻辑：缓冲区满，覆盖旧的数据

    // 先偏移索引，再存放数据
    instruction_buffer_info.head = (instruction_buffer_info.head + 1) % INSTRUCTION_BUFFER_LEN;
    instruction_buffer_info.buffer[instruction_buffer_info.head] = byte;

    instruction_buffer_info.count++;

    if (instruction_buffer_info.count > INSTRUCTION_BUFFER_LEN)
    {
        instruction_buffer_info.count = INSTRUCTION_BUFFER_LEN;
    }

    // printf("count %u\n", (u16)instruction_buffer_info.count);
}

// 往存放指令的缓冲区中存放数据，最后存放当前接收的一条指令
static void __recv_instruction_update__(u8 byte)
{
    // recv_instruction_buff[cur_recv_instruction_index++] = byte;
    recv_instruction_buff[cur_recv_instruction_index] = byte;
    cur_recv_instruction_index++;
}

void instruction_scan(void)
{
    u8 recv_byte = 0;

    if (0 == instruction_buffer_get_count())
    {
        return; // 指令缓冲区为空
    }

    recv_byte = instruction_buffer_get();

    if (INSTRUCTION_STATUS_NONE == cur_recv_instruction_status)
    {
        if (INSTRUCTION_FORMAT_HEAD == recv_byte)
        {
            cur_recv_instruction_index = 0;
            __recv_instruction_update__(INSTRUCTION_FORMAT_HEAD);

            cur_recv_instruction_status = INSTRUCTION_STATUS_FORMAT_HEAD;
        }
    }
    else if (INSTRUCTION_STATUS_FORMAT_HEAD == cur_recv_instruction_status)
    {
        switch (recv_byte)
        {
        case INSTRUCTION_TYPE_DEVICE_ON_OFF:
        {
            dest_recv_instruction_len = 4; // 要接收总共 4 个字节的命令 
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_RELAY_ON_OFF:
        {
            dest_recv_instruction_len = 5; // 要接收总共 5 个字节的命令 
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_RELAY_ACTIVE_TIME:
        {
            dest_recv_instruction_len = 6; // 要接收总共 6 个字节的命令 
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_RELAY_DEACTIVE_TIME:
        {
            dest_recv_instruction_len = 6; // 要接收总共 6 个字节的命令 
        }
        break;


        default: {} break;
        }

        __recv_instruction_update__(recv_byte);
        cur_recv_instruction_status = INSTRUCTION_STATUS_TYPE;
    }
    else if (INSTRUCTION_STATUS_TYPE == cur_recv_instruction_status)
    {
        __recv_instruction_update__(recv_byte);
        // printf("cur_recv_instruction_index %u\n", (u16)cur_recv_instruction_index);
        if (cur_recv_instruction_index >= dest_recv_instruction_len)
        {
            cur_recv_instruction_status = INSTRUCTION_STATUS_END;

            // 接收到了完整的一帧数据
            // printf("recved instruction: \n");
            // printf_buf(recv_instruction_buff, dest_recv_instruction_len);
        }
    }

    // printf_buf(recv_instruction_buff, 10); 
}

/**
 * @brief  instruction_handle() 函数退出时调用的 子函数
 *
 */
void __instruction_handle_exit__(void)
{
    // 指令有误，或者是处理完了指令，给状态机更新，让扫描函数重新接收指令
    cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
}

//

void instruction_handle(void)
{
    u8 sequencer_addr = 0x00;
    // instruction_t instruction_structure;

    if (INSTRUCTION_STATUS_END != cur_recv_instruction_status)
    {
        // 如果指令未接收完，或者是没有指令到来，直接退出
        return;
    }

    sequencer_addr = recv_instruction_buff[2]; // 存放 时序器 地址
    // instruction_structure.sequencer_addr = recv_instruction_buff[2]; // 存放 时序器 地址

    switch (recv_instruction_buff[1])
    {
    case INSTRUCTION_TYPE_DEVICE_ON_OFF:
    {
        printf("INSTRUCTION TYPE DEVICE_ON_OFF \n");
        handle_device_on_off(sequencer_addr, recv_instruction_buff[3]);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RELAY_ON_OFF:
    {
        printf("INSTRUCTION TYPE RELAY_ON_OFF \n");
        handle_relay_status_setting(sequencer_addr, recv_instruction_buff[3], recv_instruction_buff[4]);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RELAY_ACTIVE_TIME:
    {
        // 继电器 激活时间 1 ~ 999 s
        printf("INSTRUCTION_TYPE RELAY_ACTIVE_TIME \n");
        handle_relay_active_time(sequencer_addr,
            recv_instruction_buff[3],
            ((u16)recv_instruction_buff[4] << 8) | recv_instruction_buff[5]);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RELAY_DEACTIVE_TIME:
    {
        // 继电器 停用时间 1 ~ 999 s
        printf("INSTRUCTION_TYPE RELAY_DEACTIVE_TIME \n");
        handle_relay_deactive_time(sequencer_addr,
            recv_instruction_buff[3],
            ((u16)recv_instruction_buff[4] << 8) | recv_instruction_buff[5]);
    }
    break;
    // ===================================================================
    default:
    {
        printf("instruction_handle() unknown type %u\n", (u16)recv_instruction_buff[1]);
    } break;
    }

    // 指令有误，或者是处理完了指令，给状态机更新，让扫描函数重新接收指令
    cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
}

