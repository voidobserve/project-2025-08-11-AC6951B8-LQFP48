#ifndef __SEQUENCER_H__ 
#define __SEQUENCER_H__ 

#include "includes.h"
// #include "../../../apps/user_app/user_config.h"

#include "relay_handle.h"
#include "sys_time.h"


#define RELAYS_MAX 8

typedef enum
{
    DEVICE_OFF,     // 关机
    DEVICE_ON,      // 开机
}ON_OFF_FLAG;

typedef struct
{
    // ON_OFF_FLAG open_on_off;  // 继电器的开启时的开关状态（开关对应的状态）
    // ON_OFF_FLAG clod_on_off;  // 继电器的关闭时的开关状态（开关对应的状态）

    u8 last_status_on_off; // 继电器之前的状态，0--关闭，1--开启（每次设置完 cur_status_on_off，都应该更新 last_status_on_off）
    u8 cur_status_on_off; // 继电器当前状态，0--关闭，1--开启

    u16 open_time;   // 继电器开机延时时间
    u16 close_time;  // 继电器关机延时时间

    // struct sys_time countdown_open_time;   // 继电器的定时开机的时间  月 日 时 分 秒  
    // struct sys_time countdown_close_time;  // 继电器的定时关机的时间  月 日 时 分 秒  

} relay_t;

typedef struct
{
    ON_OFF_FLAG on_ff;  // 总开关

    uint8_t addr;  //设备地址
    relay_t  relay[RELAYS_MAX];  // 继电器 relay   

    // 由开关机任务来设置：
    u32 open_timeing; // 继电器开机时序的计时时间 
    u32 close_timeing; // 继电器关机时序的计时时间 

    uint8_t relay_number; // 多少路继电器 （继电器总数）

    u8 is_in_delay; // 是否处于开机或关机的延时中
} SEQUENCER_T;

enum
{
    MSG_SEQUENCER_NONE = 0x00,
    // MSG_SEQUENCER_SAVE_INFO,
    // MSG_SEQUENCER_READ_INFO,

    MSG_USER_SAVE_INFO, // 保存用户数据

    MSG_USER_SAVE_TIME, // 保存时间数据


};


/*
    定义时序器的各个状态
*/
enum
{
    SEQUENCER_STATUS_NONE = 0x00,
    SEQUENCER_STATUS_SETTING_SYS_TIME, // 设置系统时间
    SEQUENCER_STATUS_SETTING_RELAY_POWER_ON_SCHEDULE, // 设置单个继电器的定时激活计划
    SEQUENCER_STATUS_SETTING_RELAY_POWER_OFF_SCHEDULE, // 设置单个继电器的定时停用计划 
};


// extern volatile u8 flag_is_lcd_screen_on; // 标志位，lcd屏幕状态，0--未点亮，1--点亮

extern volatile u8 sequencer_status;
// extern const u8 relay_table[RELAYS_MAX];
// extern volatile ON_OFF_FLAG temp_on_off[16];  //继电器的开关
extern volatile SEQUENCER_T  sequencers;

void sequencer_relay_status_update(relay_index_t relay_index, relay_status_t relay_status);
void sequencer_relay_status_setting(relay_index_t relay_index, relay_status_t relay_status);
void sequencer_relay_status_toggle(relay_index_t relay_index);
void sequencer_relay_status_setting_dly(relay_index_t relay_index, relay_status_t relay_status, u8 delay_time);

void sequencers_data_init(void);

void user_msg_handle_task(void* p);


#endif