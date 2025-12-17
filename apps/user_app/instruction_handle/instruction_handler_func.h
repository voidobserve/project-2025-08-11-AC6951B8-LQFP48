#ifndef __INSTRUCTION_HANDLE_FUNC_H__
#define __INSTRUCTION_HANDLE_FUNC_H__
 
#include "includes.h"

void handle_device_on_off(u8 sequencer_addr, u8 cmd);
void handle_relay_status_setting(u8 sequencer_addr, u8 relay_index, u8 relay_status);
void handle_relay_active_time(u8 sequencer_addr, u8 relay_index, u16 active_time);
void handle_relay_deactive_time(u8 sequencer_addr, u8 relay_index, u16 deactive_time);

#endif