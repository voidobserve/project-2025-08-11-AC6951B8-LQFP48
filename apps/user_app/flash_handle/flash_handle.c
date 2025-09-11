#include "flash_handle.h"


#define     CFG_USER_CLOSE_SEQUENCER_DATA    3


void read_flash_sequencers_status_init(void)
{

    u8 res;
    save_flash_t save_flash3;

    memset((u8*)&save_flash3, 0, sizeof(save_flash_t));

    res = syscfg_read(CFG_USER_CLOSE_SEQUENCER_DATA, (u8*)(&save_flash3), sizeof(save_flash_t));
    if (save_flash3.header != 0x55)  //第一次上电
    {
        sequencers_data_init();
    }
    else
    {
        memcpy((u8*)(&sequencers), (u8*)(&save_flash3.seq_save), sizeof(SEQUENCER));
        printf("read sequencers data\n");
    }

    printf("read_flash_sequencers_status_init\n");

    printf("sequencers.on_ff = %u\n", (u16)sequencers.on_ff);

}

void save_sequencers_data_area3(void)
{
    save_flash_t save_data;
    save_data.header = 0x55;
    memcpy((u8*)(&save_data.seq_save), (u8*)(&sequencers), sizeof(SEQUENCER));
    syscfg_write(CFG_USER_CLOSE_SEQUENCER_DATA, (u8*)(&save_data), sizeof(save_flash_t));
    printf("save sequencers data arae3 successed \n");
    printf("sequencers.on_ff = %u\n", (u16)sequencers.on_ff);
}

