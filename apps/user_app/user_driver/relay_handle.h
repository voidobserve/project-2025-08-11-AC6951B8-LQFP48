#ifndef __RELAY_HANDLE_H__
#define __RELAY_HANDLE_H__

#include "includes.h" 
 
/*
    定义 驱动继电器的引脚
    例如 索引值0 对应引脚 PA06

    驱动继电器的引脚又与按键灯相连，控制继电器时也会控制对应的按键灯
*/
#define RELAY_PIN_INDEX_0 IO_PORTA_06
#define RELAY_PIN_INDEX_1 IO_PORTA_05
#define RELAY_PIN_INDEX_2 IO_PORTA_04
#define RELAY_PIN_INDEX_3 IO_PORTA_03
#define RELAY_PIN_INDEX_4 IO_PORTA_02
#define RELAY_PIN_INDEX_5 IO_PORTA_01
#define RELAY_PIN_INDEX_6 IO_PORTA_00
#define RELAY_PIN_INDEX_7 IO_PORTC_07
typedef unsigned int relay_pin_index_t;  // u32 -- relay_pin_index_t
/*
    定义继电器索引

    用于给 relay_index_t 类型的变量赋值
*/
typedef enum 
{
    RELAY_INDEX_0 = 0,
    RELAY_INDEX_1,
    RELAY_INDEX_2,
    RELAY_INDEX_3,
    RELAY_INDEX_4,
    RELAY_INDEX_5,
    RELAY_INDEX_6,
    RELAY_INDEX_7,
}  relay_index_t;

// 定义继电器 激活和不激活时对应引脚的电平
typedef enum
{
    RELAY_STATUS_DEACTIVE = 0,// 继电器 不激活（关闭、断开）
    RELAY_STATUS_ACTIVE = 1, // 继电器 激活（启动、闭合）
} relay_status_t;


void relay_status_init(void); // 继电器状态初始化，默认为不激活的状态
void relay_status_setting(relay_index_t relay_index, relay_status_t relay_status); // 设置继电器的状态，会直接操作继电器
void relay_status_toggle(relay_index_t relay_index); //  翻转 继电器的状态 ， 会直接操作继电器
void relay_status_setting_dly(relay_index_t relay_index, relay_status_t relay_status, u8 delay_time);

relay_status_t relay_status_get(relay_index_t relay_index);


#endif

