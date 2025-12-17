#ifndef __SEQUENCER_DEVICE_ON_OFF_H__
#define __SEQUENCER_DEVICE_ON_OFF_H__

#include "includes.h"
#include "sequencer.h"

// #define SEQUENCER_POWER_ON_TASK_NAME ((unsigned char *)"sequencer_power_on_task")

// void sequencer_update_max_power_time_before_first_power_on(void);

u8 is_sequencer_in_delay(void); // 确认 时序器是否处于开关机的延时状态
void sequencer_flag_in_delay_set(void); // 置位 时序器的开关机延时状态标志
void sequencer_flag_in_delay_clear(void); // 清除 时序器的开关机延时状态标志


// void sequencer_first_power_on(void);
void sequencer_power_on(void);
void sequencer_power_off(void);

#endif