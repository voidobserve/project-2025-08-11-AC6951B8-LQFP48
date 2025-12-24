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
};

// 定义各个不同的指令
typedef enum
{
    INSTRUCTION_TYPE_NONE = 0x00,

    INSTRUCTION_TYPE_DEVICE_ON_OFF = 0x01, // 设备开关机
    INSTRUCTION_TYPE_RELAY_ON_OFF = 0x02, // 单个继电器开关 

    INSTRUCTION_TYPE_RELAY_ACTIVE_TIME = 0x03, // 单个继电器 激活 延时（开启延时）
    INSTRUCTION_TYPE_RELAY_DEACTIVE_TIME = 0x04, // 单个继电器 停用 延时（关闭延时）

    INSTRUCTION_TYPE_SET_SYS_TIME = 0x05,// 设置系统时间

    INSTRUCTION_TYPE_SET_TIME_TO_SWITCH_ON_OFF = 0x06, // 设置对应星期值的定时开关机（每周对应星期值的开关机计划）
    INSTRUCTION_TYPE_CANCEL_TIME_TO_SWITCH_ON_OFF = 0x07, // 取消对应星期值的开关机（取消每周对应星期值的开关机计划）

    INSTRUCTION_TYPE_INIT_ALL_DEVICE_ADDR = 0x08, // 初始化所有时序器设备的地址

    // 恢复出厂设置
    INSTRUCTION_TYPE_RESET_TO_FACTORY_SETTING = 0xFF, 
} instruction_type_t;


u16 instruction_buffer_get_count(void);
u8 instruction_buffer_get(void);
void instruction_buffer_put(u8 byte);

void instruction_scan(void);
void instruction_handle(void);


#endif