#ifndef __INSTRUCTION_HANDLE_H 
#define __INSTRUCTION_HANDLE_H 

#include "includes.h"
 

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

    INSTRUCTION_TYPE_DEVICE_ON_OFF = 0x01, // 设备开关机
    INSTRUCTION_TYPE_RELAY_ON_OFF = 0x02, // 单个继电器开关 

    INSTRUCTION_TYPE_RELAY_ACTIVE_TIME = 0x03, // 单个继电器激活延时（开启延时）
    INSTRUCTION_TYPE_RELAY_DEACTIVE_TIME = 0x04, // 单个继电器不激活延时（关闭延时）

};



u16 instruction_buffer_get_count(void);
u8 instruction_buffer_get(void);
void instruction_buffer_put(u8 byte);

void instruction_scan(void);
void instruction_handle(void);


#endif