#include "sequencer.h"

volatile u8 flag_is_lcd_screen_on = 0; // 标志位，lcd屏幕状态，0--未点亮，1--点亮

volatile ON_OFF_FLAG temp_on_off[16];  //继电器的开关

SEQUENCER  sequencers;
