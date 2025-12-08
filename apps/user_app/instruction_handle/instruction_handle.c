#include "instruction_handle.h" 


#define INSTRUCTION_BUFFER_LEN ((u16) 128) // 环形缓冲区长度

// 定义环形缓冲区结构体类型：
typedef struct
{
    u8 buffer[INSTRUCTION_BUFFER_LEN];
    u8 head;
    u8 tail;
    u16 count;
} instruction_buffer_info_t;

static volatile instruction_buffer_info_t instruction_buffer_info;

static volatile u8 cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
static volatile u8 cur_recv_instruction_index = 0;
static volatile u8 dest_recv_instruction_len = 0; // 记录当前要接收的数据指令长度

// 存放当前接收到的一条指令
volatile u8 recv_instruction_buff[20] = { 0 };

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

void recv_instruction_update(u8 byte)
{
    // recv_instruction_buff[cur_recv_instruction_index++] = byte;
    recv_instruction_buff[cur_recv_instruction_index] = byte;
    cur_recv_instruction_index++;
}

void instruction_handle(void)
{
    u8 recv_byte = 0;

    if (0 == instruction_buffer_get_count())
    {
        return;
    }

    recv_byte = instruction_buffer_get();

    if (INSTRUCTION_STATUS_NONE == cur_recv_instruction_status)
    {
        if (INSTRUCTION_FORMAT_HEAD == recv_byte)
        {
            cur_recv_instruction_index = 0;
            recv_instruction_update(INSTRUCTION_FORMAT_HEAD);

            cur_recv_instruction_status = INSTRUCTION_STATUS_FORMAT_HEAD;
        }
    }
    else if (INSTRUCTION_STATUS_FORMAT_HEAD == cur_recv_instruction_status)
    {
        switch (recv_byte)
        {
        case INSTRUCTION_TYPE_RELAY_ON_OFF:
        {
            dest_recv_instruction_len = 5; // 要接收总共 5 个字节的命令 
        }
        break;


        default:

            break;
        }

        recv_instruction_update(recv_byte);
        cur_recv_instruction_status = INSTRUCTION_STATUS_TYPE;
    }
    else if (INSTRUCTION_STATUS_TYPE == cur_recv_instruction_status)
    {
        recv_instruction_update(recv_byte);
        // printf("cur_recv_instruction_index %u\n", (u16)cur_recv_instruction_index);
        if (cur_recv_instruction_index >= dest_recv_instruction_len)
        {
            cur_recv_instruction_status = INSTRUCTION_STATUS_END;
        }
    }


    if (INSTRUCTION_STATUS_END == cur_recv_instruction_status)
    {
        switch (recv_instruction_buff[1])
        {
        case INSTRUCTION_TYPE_RELAY_ON_OFF:
        {
            printf("INSTRUCTION_TYPE_RELAY_ON_OFF \n");
            // extern void printf_buf(u8 * buf, u32 len);
            // printf_buf(recv_instruction_buff, dest_recv_instruction_len);

            // 如果 设备ID 是指定所有设备
            if (0xFF == recv_instruction_buff[2])
            {
                if (recv_instruction_buff[3] > 8 || 0 == recv_instruction_buff[3])
                {
                    // 继电器的索引 超过了 继电器的数量
                    return;
                }

                if (0x01 == recv_instruction_buff[4])
                {
                    relay_status_setting(recv_instruction_buff[3] - 1, RELAY_STATUS_ACTIVE);
                    lcd_relay_icon_show(recv_instruction_buff[3] - 1);
                }
                else if (0x00 == recv_instruction_buff[4])
                {
                    relay_status_setting(recv_instruction_buff[3] - 1, RELAY_STATUS_DEACTIVE);
                    lcd_relay_icon_unshow(recv_instruction_buff[3] - 1);
                }
            }
        }
        break;


        default:

            break;
        }

        cur_recv_instruction_status = INSTRUCTION_STATUS_NONE;
    }

    // printf_buf(recv_instruction_buff, 10);

}

