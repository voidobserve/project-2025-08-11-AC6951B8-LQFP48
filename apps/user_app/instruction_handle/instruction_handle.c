#include "instruction_handle.h" 
#include "user_config.h"
#include "user_sys_time.h"
#include "instruction_handler_func.h"
#include "instruction_feedback.h"
#include "instruction_transmit.h"


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
static volatile u16 instruction_buffer_timeout = 0; // 指令接收超时计数值

// 存放当前接收到的一条指令：
volatile u8 recv_instruction_buff[20] = { 0 };
volatile u8 recv_instruction_buff_len = 0; // 存放当前接收完成的指令的长度 

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

// 累计指令接收超时的时间
void instruction_buffer_timeout_add(void)
{
    if (instruction_buffer_timeout < 65535)
    {
        // 防止计数溢出
        instruction_buffer_timeout++;
    }
}

// 清除指令接收超时的时间
void instruction_buffer_timeout_reset(void)
{
    instruction_buffer_timeout = 0;
}

int instruction_buffer_is_timeout(void)
{
    // 注意，这里的超时时间应该比串口接收超时时间还要长一些
    /*
        instruction_buffer_timeout 的时间单位由
        调用函数 instruction_buffer_timeout_add() 的时间周期决定
    */
    if (instruction_buffer_timeout > 20)
    {
        return 1;
    }
    else
    {
        return 0;
    }
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

    if (cur_recv_instruction_status != INSTRUCTION_STATUS_NONE)
    {
        // 当前并不处于 等待接收完整的一帧指令的状态，累计接收超时时间
        instruction_buffer_timeout_add();
        if (instruction_buffer_is_timeout())
        {
            // 累计的接收超时时间超过一定值，则认为接收指令超时
            instruction_buffer_timeout_reset();
            cur_recv_instruction_status = INSTRUCTION_STATUS_NONE; // 给状态机更新，重新接收指令        
#if USER_DEBUG_ENABLE
            printf("instruction recv timeout\n");
#endif
        }
    }

    if (0 == instruction_buffer_get_count())
    {
        return; // 指令缓冲区为空
    }

    recv_byte = instruction_buffer_get();
    instruction_buffer_timeout_reset(); // 有新数据，重置接收超时计数

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
        case INSTRUCTION_TYPE_SET_RELAY_SCHEDULE:
        {
            /*
                总共要接收 11 个字节的命令
                格式头 + 指令类型 + 设备地址 +
                继电器编号(1byte) +
                星期(1byte) +
                小时(1byte) + 分钟(1byte) + 秒(1byte) +
                小时(1byte) + 分钟(1byte) + 秒(1byte)
            */
            dest_recv_instruction_len = 11;
        }
        break;
        // ============================================================================
        case INSTRUCTION_TYPE_CANCEL_RELAY_SCHEDULE:
        {
            // 总共要接收 5 个字节的命令
            dest_recv_instruction_len = 5;
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
            // 接收到了完整的一帧数据
            cur_recv_instruction_status = INSTRUCTION_STATUS_END;
            recv_instruction_buff_len = dest_recv_instruction_len;

#if USER_DEBUG_ENABLE
            printf("\n==========================================\n");
            printf("recved instruction: \n");
            printf_buf(recv_instruction_buff, dest_recv_instruction_len);
            printf("\n==========================================\n");
#endif 
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
    // u8 flag_is_need_to_transmit = 0; // 标志位，是否要将指令传给下位机
    u8 sequencer_addr = 0x00;
    u8 instruction_type = 0x00;
    // instruction_t instruction_structure;

    if (INSTRUCTION_STATUS_END != cur_recv_instruction_status)
    {
        // 如果指令未接收完，或者是没有指令到来，直接退出
        return;
    }

    instruction_type = recv_instruction_buff[1]; // 存放 指令类型
    sequencer_addr = recv_instruction_buff[2]; // 存放 时序器 地址
    // instruction_structure.sequencer_addr = recv_instruction_buff[2]; // 存放 时序器 地址

    switch (instruction_type)
    {
    case INSTRUCTION_TYPE_DEVICE_ON_OFF:
    {
#if USER_DEBUG_ENABLE
        printf("INSTRUCTION_TYPE_DEVICE_ON_OFF \n");
#endif
        ret = handle_device_on_off(sequencer_addr, recv_instruction_buff[3]);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RELAY_ON_OFF:
    {
#if USER_DEBUG_ENABLE
        printf("INSTRUCTION TYPE RELAY_ON_OFF \n");
#endif
        ret = handle_relay_status_setting(sequencer_addr, recv_instruction_buff[3], recv_instruction_buff[4]);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RELAY_ACTIVE_TIME:
    {
        // 继电器 激活时间 1 ~ 999 s
#if USER_DEBUG_ENABLE
        printf("INSTRUCTION_TYPE RELAY_ACTIVE_TIME \n");
#endif
        ret = handle_relay_active_time(sequencer_addr,
            recv_instruction_buff[3],
            ((u16)recv_instruction_buff[4] << 8) | recv_instruction_buff[5]);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RELAY_DEACTIVE_TIME:
    {
        // 继电器 停用时间 1 ~ 999 s
#if USER_DEBUG_ENABLE
        printf("INSTRUCTION_TYPE RELAY_DEACTIVE_TIME \n");
#endif
        ret = handle_relay_deactive_time(sequencer_addr,
            recv_instruction_buff[3],
            ((u16)recv_instruction_buff[4] << 8) | recv_instruction_buff[5]);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_SET_SYS_TIME:
    {
        // 设置系统时间
        printf("INSTRUCTION_TYPE SET_SYS_TIME \n");
        user_sys_time_t time = { 0 };
        time.year = ((u16)recv_instruction_buff[3] << 8) | recv_instruction_buff[4];
        time.month = recv_instruction_buff[5];
        time.day = recv_instruction_buff[6];
        time.hour = recv_instruction_buff[7];
        time.min = recv_instruction_buff[8];
        time.sec = recv_instruction_buff[9];
        ret = handle_set_sys_time(sequencer_addr, time);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_SET_TIME_TO_SWITCH_ON_OFF:
    {
        // 设置每周对应星期值的开关机计划
#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE SET_TIME_TO_SWITCH_ON_OFF \n");
#endif
        user_sys_time_t power_on_time = { 0 };
        user_sys_time_t power_off_time = { 0 };
        u8 weekday = 255;

        weekday = recv_instruction_buff[3]; // 星期x 
        power_on_time.hour = recv_instruction_buff[4];
        power_on_time.min = recv_instruction_buff[5];
        power_on_time.sec = recv_instruction_buff[6];

        power_off_time.hour = recv_instruction_buff[7];
        power_off_time.min = recv_instruction_buff[8];
        power_off_time.sec = recv_instruction_buff[9];

        ret = handle_set_weekly_schedule(sequencer_addr, weekday, power_on_time, power_off_time);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_CANCEL_TIME_TO_SWITCH_ON_OFF:
    {
        // 取消每周对应星期值的开关机计划
#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE CANCEL_TIME_TO_SWITCH_ON_OFF \n");
#endif

        u8 weekday = 255;
        weekday = recv_instruction_buff[3];
        // printf("recv weekday %u\n", (u16)weekday);
        printf_buf(recv_instruction_buff, 4);
        ret = handle_cancel_weekly_schedule(sequencer_addr, weekday);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_INIT_ALL_DEVICE_ADDR:
    {
        // 初始化所有时序器设备的地址
        if (recv_instruction_buff[3] != 0x5C)
        {
            ret = 1;
            break; // 提前退出当前swtich-case语句
        }

        ret = handle_init_all_device_addr(sequencer_addr);
        // printf("ret == %d\n", ret);

        // 需要给下位机转发指令
        if (sequencer_addr != 0xFF &&
            (sequencer_addr + 1) != 0xFF)
        {
            // 如果不是0xFF广播地址，则转发指令给下位机
            recv_instruction_buff[2] = sequencer_addr + 1; // 存放下位机的地址
            // 整个指令长度为 4 个字节
            instruction_transmit_buff(recv_instruction_buff, 4);
        }
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_RESET_TO_FACTORY_SETTING:
    {
        // 恢复出厂设置
#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE RESET_TO_FACTORY_SETTING \n");
#endif
        if (recv_instruction_buff[3] != 0x5C)
        {
            ret = 1;
            break; // 提前退出当前swtich-case语句
        }

        ret = handle_reset_to_factory_setting(sequencer_addr);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_SET_RELAY_SCHEDULE:
    {
        // 设置某个继电器每周对应星期值的定时激活、定时停用计划
        user_sys_time_t active_time = { 0 };// 存放定时激活时间
        user_sys_time_t deactive_time = { 0 }; // 存放定时停用时间
        relay_index_t relay_index = RELAY_INDEX_INVALID;
        u8 weekday = 255;

#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE SET_RELAY_SCHEDULE \n");
#endif

        relay_index = recv_instruction_buff[3];
        weekday = recv_instruction_buff[4]; // 星期值
        active_time.hour = recv_instruction_buff[5];
        active_time.min = recv_instruction_buff[6];
        active_time.sec = recv_instruction_buff[7];

        deactive_time.hour = recv_instruction_buff[8];
        deactive_time.min = recv_instruction_buff[9];
        deactive_time.sec = recv_instruction_buff[10];
        ret = handle_set_relay_weekly_schedule(sequencer_addr, relay_index, weekday, active_time, deactive_time);
        // printf("ret == %d\n", ret);
    }
    break;
    // ===================================================================
    case INSTRUCTION_TYPE_CANCEL_RELAY_SCHEDULE:
    {
        // 取消某个继电器对应星期值的定时激活、定时停用计划
        relay_index_t relay_index = RELAY_INDEX_INVALID;
        u8 weekday = 255;

#if USER_DEBUG_ENABLE        
        printf("INSTRUCTION_TYPE CANCEL_RELAY_SCHEDULE \n");
#endif

        relay_index = recv_instruction_buff[3]; // 继电器编号（注意数值是1~8）
        weekday = recv_instruction_buff[4]; // 星期值
        ret = handle_cancel_relay_weekly_schedule(sequencer_addr, relay_index, weekday);
        printf("ret == %d\n", ret);
    }
    break;
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

    if (sequencer_addr == sequencers.addr || sequencer_addr == 0xFF)
    {
        // 如果地址匹配，或者地址为0xFF，则反馈指令结果
        if (ret)
        {
            // 如果指令出错
            instruction_feedback_fail(sequencers.addr, instruction_type);
        }
        else
        {
            // 如果指令未出错
            instruction_feedback_success(sequencers.addr, instruction_type);
            os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO); // 保存数据到flash
        }
    }

    if ((instruction_type != INSTRUCTION_TYPE_INIT_ALL_DEVICE_ADDR) &&
        sequencer_addr != sequencers.addr || sequencer_addr == 0xFF)
    {
        // 如果地址不匹配，或者地址为0xFF，将收到的指令转发给下位机
        instruction_transmit_buff(recv_instruction_buff, recv_instruction_buff_len);
    }

    // 指令有误，或者是处理完了指令，给状态机更新，让扫描函数重新接收指令
    cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
}

