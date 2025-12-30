#ifndef __FLASH_HANDLE_H__
#define __FLASH_HANDLE_H__

#include "includes.h"
#include "../../../apps/user_app/sequencer/sequencer.h"
#include "user_schedule.h"


#pragma pack (1)

// @attention 保存数据结构体中，单个结构体变量的大小不能大于255字节，否则保存和读取会失败
// USER_TO_DO 这里的数据加起来已经超过了 512 字节，需要换成其他的接口
typedef struct
{
    unsigned char header;           //头部 判断数据是否第一次写入  
    SEQUENCER_T seq_save;

    user_sys_time_t sys_time; // 存放 系统时间 

    weekly_schedule_t weekly_schedule; // 存放时序器的定时开关机时间 
    weekly_schedule_t weekly_schedule_relay[8]; // 存放 继电器的定时开关机时间
    
}save_flash_t;


#pragma pack ()

// 需要保存数据时，延时保存的时间：（单位：ms）
#define DELAY_SAVE_FLASH_TIMES ((u16)3000)

// extern void read_flash_sequencers_status_init(void); 
// extern void save_sequencers_data_area3(void);

void save_user_data_init(void);
void save_user_data_area3(void);

void save_user_data_enable(void); // 使能保存数据的倒计时，使能保存数据的操作
void save_user_data_time_count_down(void);// 保存数据的倒计时
void save_user_data_handle(void); // 保存用户数据，内部会根据标志位来判断是否保存

#endif