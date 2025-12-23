#include "instruction_handle.h" 
#include "user_config.h"
#include "user_sys_time.h"
#include "instruction_handler_func.h"
#include "instruction_feedback.h"


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
        // ============================================================================
        case INSTRUCTION_TYPE_SET_SYS_TIME:
        {
            /*
                要接收总共 10 个字节的命令
                格式头 + 指令类型 + 设备地址 +
                年份(2byte) + 月份(1byte) + 日(1byte) +
                小时(1byte) + 分钟(1byte) + 秒(1byte)
            */
            dest_recv_instruction_len = 10;
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_SET_TIME_TO_SWITCH_ON_OFF:
        {
            /*
                要接收总共 10 个字节的命令
                格式头 + 指令类型 + 设备地址 +
                星期(1byte) +
                小时(1byte) + 分钟(1byte) + 秒(1byte) +
                小时(1byte) + 分钟(1byte) + 秒(1byte)
            */
            dest_recv_instruction_len = 10;
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_CANCEL_TIME_TO_SWITCH_ON_OFF:
        {
            // 总共要接收 4 个字节的命令 
            dest_recv_instruction_len = 4;
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_INIT_ALL_DEVICE_ADDR:
        {
            // 总共要接收 4 个字节的命令 
            dest_recv_instruction_len = 4;
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_RESET_TO_FACTORY_SETTING:
        {
            // 总共要接收 4 个字节的命令 
            dest_recv_instruction_len = 4;
        }
        break;
        // ============================================================================
        default:
        {
            dest_recv_instruction_len = 0;
        } break;
        }

#if USER_DEBUG_ENABLE
        printf("recv type %u\n", (u16)recv_byte);
#endif
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
    int ret = 0; // 接收函数的返回值
    u8 sequencer_addr = 0x00;
    u8 instruction_type = 0x00;
    // instruction_t instruction_structure;

    if (INSTRUCTION_STATUS_END != cur_recv_instruction_status)
    {
        // 如果指令未接收完，或者是没有指令到来，直接退出
        return;
    }

    sequencer_addr = recv_instruction_buff[2]; // 存放 时序器 地址
    // instruction_structure.sequencer_addr = recv_instruction_buff[2]; // 存放 时序器 地址
    instruction_type = recv_instruction_buff[1];

    switch (instruction_type)
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
    case INSTRUCTION_TYPE_SET_SYS_TIME:
    {
        printf("INSTRUCTION_TYPE SET_SYS_TIME \n");
        user_sys_time_t time = { 0 };
        time.year = ((u16)recv_instruction_buff[3] << 8) | recv_instruction_buff[4];
        time.month = recv_instruction_buff[5];
        time.day = recv_instruction_buff[6];
        time.hour = recv_instruction_buff[7];
        time.min = recv_instruction_buff[8];
        time.sec = recv_instruction_buff[9];
        ret = handle_set_sys_time(sequencer_addr, time);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_SET_TIME_TO_SWITCH_ON_OFF:
    {
        user_sys_time_t power_on_time = { 0 };
        user_sys_time_t power_off_time = { 0 };
        u8 weekday = 255;
#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE SET_TIME_TO_SWITCH_ON_OFF \n");
#endif

        weekday = recv_instruction_buff[3]; // 星期x 
        power_on_time.hour = recv_instruction_buff[4];
        power_on_time.min = recv_instruction_buff[5];
        power_on_time.sec = recv_instruction_buff[6];

        power_off_time.hour = recv_instruction_buff[7];
        power_off_time.min = recv_instruction_buff[8];
        power_off_time.sec = recv_instruction_buff[9];

        handle_set_weekly_schedule(sequencer_addr, weekday, power_on_time, power_off_time);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_CANCEL_TIME_TO_SWITCH_ON_OFF:
    {
#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE CANCEL_TIME_TO_SWITCH_ON_OFF \n");
#endif

        u8 weekday = 255;
        weekday = recv_instruction_buff[3];
        // printf("recv weekday %u\n", (u16)weekday);
        printf_buf(recv_instruction_buff, 4);
        ret = handle_cancel_weekly_schedule(sequencer_addr, weekday);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_INIT_ALL_DEVICE_ADDR:
    {
        if (recv_instruction_buff[3] != 0x5C)
        {
            ret = 1;
            break; // 提前退出当前swtich-case语句
        }

        handle_init_all_device_addr(sequencer_addr);

        // 需要给下位机转发指令
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RESET_TO_FACTORY_SETTING:
    {
#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE RESET_TO_FACTORY_SETTING \n");
#endif
        if (recv_instruction_buff[3] != 0x5C)
        {
            ret = 1;
            break; // 提前退出当前swtich-case语句
        }

        ret = handle_reset_to_factory_setting(sequencer_addr);
    }
    break;
    // ===================================================================
    // ===================================================================
    default:
    {
#if USER_DEBUG_ENABLE        
        printf("%s unknown type: %u\n", __func__, (u16)recv_instruction_buff[1]);
#endif
        ret = 1;
        // instruction_feedback_fail(sequencers.addr, instruction_type);
    } break;
    }

    if (ret)
    {
        // 如果指令出错
        instruction_feedback_fail(sequencers.addr, instruction_type);
    }
    else
    {
        // 如果指令未出错
        instruction_feedback_success(sequencers.addr, instruction_type);
    }

    // 指令有误，或者是处理完了指令，给状态机更新，让扫描函数重新接收指令
    cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
}

