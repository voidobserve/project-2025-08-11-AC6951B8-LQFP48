#ifndef __INSTRUCTION_HANDLE_H 
#define __INSTRUCTION_HANDLE_H 

#include "includes.h"

#include "../../../apps/user_app/user_config.h"
 
#define INSTRUCTION_FORMAT_HEAD ((u8)0xC5)

// 定义接收指令所用的状态：
enum
{
    INSTRUCTION_STATUS_NONE = 0x00,
    INSTRUCTION_STATUS_FORMAT_HEAD,
    INSTRUCTION_STATUS_TYPE, // 控制命令
    INSTRUCTION_STATUS_END,
};

// 定义各个不同的指令
enum
{
    INSTRUCTION_TYPE_NONE = 0x00,
    INSTRUCTION_TYPE_RELAY_ON_OFF = 0x01 , // 继电器开关 
};


 
u16 instruction_buffer_get_count(void);
u8 instruction_buffer_get(void);
void instruction_buffer_put(u8 byte);

void instruction_handle(void);
 

#endif