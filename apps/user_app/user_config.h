#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "includes.h" // 包含芯片官方提供的头文件

#include "../../apps/user_app/user_driver/power_light.h" // 电源指示灯

#include "../../apps/user_app/lcd/lcd1621.h" // lcd1621
#include "../../apps/user_app/user_driver/user_uart_driver.h"
#include "../../apps/user_app/user_driver/user_uart1_driver.h" // 串口1 驱动
#include "../../apps/user_app/user_driver/relay_handle.h" // 继电器 驱动

#include "../../apps/user_app/sequencer/sequencer.h" 
#include "../../apps/user_app/sequencer/sequencer_device_on_off.h"

#include "../../apps/user_app/flash_handle/flash_handle.h"

#include "../../apps/user_app/ac_detection/ac_detection.h"

#include "user_main_task.h"
#include "instruction_handle/instruction_handle.h"





#endif