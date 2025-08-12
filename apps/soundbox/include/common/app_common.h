
#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#include "typedef.h"
#include "asm/uart_dev.h"

#pragma pack (1)
typedef struct 
{
    unsigned char header;           //头部 判断数据是否第一次写入  
    SEQUENCER seq_save;
   // base_ins_t 
}save_flash_t;
#pragma pack ()



extern u8 app_common_key_var_2_event(u32 key_var);

#endif
