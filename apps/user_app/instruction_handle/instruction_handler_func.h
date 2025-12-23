#ifndef __INSTRUCTION_HANDLE_FUNC_H__
#define __INSTRUCTION_HANDLE_FUNC_H__
 
#include "includes.h"
#include "user_sys_time.h"

void handle_device_on_off(u8 sequencer_addr, u8 cmd);
void handle_relay_status_setting(u8 sequencer_addr, u8 relay_index, u8 relay_status);
void handle_relay_active_time(u8 sequencer_addr, u8 relay_index, u16 active_time);
void handle_relay_deactive_time(u8 sequencer_addr, u8 relay_index, u16 deactive_time);
int handle_set_sys_time(u8 sequencer_addr, user_sys_time_t time); // 收到对应的串口指令后，设置系统时间
void handle_set_weekly_schedule(u8 sequencer_addr, u8 weekday, user_sys_time_t power_on_time, user_sys_time_t power_off_time);
int handle_cancel_weekly_schedule(u8 sequencer_addr, u8 weekday);
void handle_init_all_device_addr(u8 sequencer_addr);

int handle_reset_to_factory_setting(u8 sequencer_addr);

#endif