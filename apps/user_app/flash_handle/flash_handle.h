#ifndef __FLASH_HANDLE_H
#define __FLASH_HANDLE_H 

#include "includes.h"
#include "../../../apps/user_app/sequencer/sequencer.h"

#pragma pack (1)
typedef struct
{
    unsigned char header;           //头部 判断数据是否第一次写入  
    SEQUENCER seq_save;
    // base_ins_t 
}save_flash_t;
#pragma pack ()

extern void read_flash_sequencers_status_init(void); 
extern void save_sequencers_data_area3(void);

#endif