#ifndef __SEQUENCER_DEVICE_ON_OFF_H
#define __SEQUENCER_DEVICE_ON_OFF_H 

#include "includes.h"
#include "sequencer.h"

// #define SEQUENCER_POWER_ON_TASK_NAME ((unsigned char *)"sequencer_power_on_task")

// void sequencer_update_max_power_time_before_first_power_on(void);
void sequencer_update_max_power_on_time(void);

void sequencer_update_max_power_off_time(void);

void sequencer_first_power_on(void);
void sequencer_power_on(void);
void sequencer_power_off(void);

#endif