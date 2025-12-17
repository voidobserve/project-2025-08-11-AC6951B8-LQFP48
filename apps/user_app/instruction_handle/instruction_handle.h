#ifndef __INSTRUCTION_HANDLE_H__ 
#define __INSTRUCTION_HANDLE_H__ 

#include "includes.h"
 

#define INSTRUCTION_FORMAT_HEAD ((u8)0xC5)

// 定义接收指令所用的状态：
typedef enum
{
    INSTRUCTION_STATUS_NONE = 0x00,
    INSTRUCTION_STATUS_FORMAT_HEAD,
    INSTRUCTION_STATUS_TYPE, // 控制命令
    INSTRUCTION_STATUS_END,
} ;

// 定义各个不同的指令
typedef enum
{
    INSTRUCTION_TYPE_NONE = 0x00,

    INSTRUCTION_TYPE_DEVICE_ON_OFF = 0x01, // 设备开关机
    INSTRUCTION_TYPE_RELAY_ON_OFF = 0x02, // 单个继电器开关 

    INSTRUCTION_TYPE_RELAY_ACTIVE_TIME = 0x03, // 单个继电器 激活 延时（开启延时）
    INSTRUCTION_TYPE_RELAY_DEACTIVE_TIME = 0x04, // 单个继电器 停用 延时（关闭延时）

} instruction_type_t ;

// typedef struct
// {
//     u8 instruction_type; // 指令类型
//     u8 sequencer_addr; // 指令中的时序器地址
//     u8 cmd; // 指令中的命令
//     u8 data[10];
// } instruction_t;

u16 instruction_buffer_get_count(void);
u8 instruction_buffer_get(void);
void instruction_buffer_put(u8 byte);

void instruction_scan(void);
void instruction_handle(void);


#endif