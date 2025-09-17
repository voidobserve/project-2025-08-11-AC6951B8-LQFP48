#ifndef __SEQUENCER_H 
#define __SEQUENCER_H 

#include "includes.h"
#include "../../include_lib/system/sys_time.h"




#define RELAYS_MAX 8

typedef enum
{
    DEVICE_OFF,     // 关机
    DEVICE_ON,      // 开机
}ON_OFF_FLAG;



typedef struct
{
    ON_OFF_FLAG open_on_off;  // 继电器的开启时的开关状态（开关对应的状态）
    ON_OFF_FLAG clod_on_off;  // 继电器的关闭时的开关状态（开关对应的状态）

    u8 last_status_on_off; // 继电器之前的状态，0--关闭，1--开启（每次设置完 cur_status_on_off，都应该更新 last_status_on_off）
    u8 cur_status_on_off; // 继电器当前状态，0--关闭，1--开启

    uint8_t open_time;   // 继电器开机延时时间
    uint8_t close_time;  // 继电器关机延时时间
    struct sys_time countdown_open_time;   // 继电器的定时开机的时间  月 日 时 分 秒  
    struct sys_time countdown_close_time;  // 继电器的定时关机的时间  月 日 时 分 秒  

}RELAYS;

typedef struct
{
    ON_OFF_FLAG on_ff;  // 总开关

    uint8_t addr;  //设备地址
    RELAYS  realy[RELAYS_MAX];  // 继电器 relay（原本的工程中是realy）
    uint8_t timeing_flag;  // 0:计时中 1：计时结束
    // uint8_t open_timeing;   // 开机时序的计时时间
    u16 open_timeing; // 继电器开机时序的计时时间
    // uint8_t close_timeing;   // 关机时序而定计时时间
    u16 close_timeing; // 继电器关机时序的计时时间
    uint8_t relay_number;    //多少路继电器 （继电器总数）
}SEQUENCER;

enum
{
    MSG_SEQUENCER_NONE = 0x00,
    MSG_SEQUENCER_SAVE_INFO,
    // MSG_SEQUENCER_READ_INFO,
};

extern volatile u8 flag_is_lcd_screen_on; // 标志位，lcd屏幕状态，0--未点亮，1--点亮

extern const u8 relay_table[RELAYS_MAX];
extern volatile ON_OFF_FLAG temp_on_off[16];  //继电器的开关
extern volatile SEQUENCER  sequencers;


extern void relay_off_on(u32 relay_led, u8 relay_number);
void user_msg_handle_task(void *p);


#endif