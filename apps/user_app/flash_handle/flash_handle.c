#include "flash_handle.h"
#include "syscfg_id.h"
#include "sequencer.h"
#include "user_schedule.h"

/*
    使用说明：

    save_user_data_time_count_down() 10ms调用一次，不需要特别准确
    save_user_data_handle() 放到主循环，一直调用

    save_user_data_enable() 放到用户消息处理的线程中，有消息到来便调用

    调用 os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    给 用户消息处理的线程 发送消息
*/

#define FLASH_CRC_DATA 0xC5

static volatile u16 time_count_down = 0; // 存放当前的倒计时
static volatile u8 flag_is_enable_count_down = 0;
static volatile u8 flag_is_enable_to_save = 0; // 标志位，是否使能了保存


void save_user_data_init(void)
{
    u8 res;
    save_flash_t save_flash3;

    memset((u8*)&save_flash3, 0, sizeof(save_flash_t));

    res = syscfg_read(CFG_USER_CLOSE_SEQUENCER_DATA, (u8*)(&save_flash3), sizeof(save_flash_t));
    if (save_flash3.header != FLASH_CRC_DATA)  //第一次上电
    {
        sequencers_data_init();

#if 1 
        extern volatile weekly_schedule_t weekly_schedule;
        memcpy(&weekly_schedule, &save_flash3.weekly_schedule, sizeof(weekly_schedule_t));
#endif
    }
    else
    {
        memcpy((u8*)(&sequencers), (u8*)(&save_flash3.seq_save), sizeof(SEQUENCER_T));
        printf("read sequencers data\n");
    }

    printf("__FUN__ %s\n", __func__);

    printf("sequencers.on_ff = %u\n", (u16)sequencers.on_ff);
}

void save_user_data_area3(void)
{
    save_flash_t save_data;
    save_data.header = FLASH_CRC_DATA;
    memcpy((u8*)(&save_data.seq_save), (u8*)(&sequencers), sizeof(SEQUENCER_T));

#if 1  
    extern volatile weekly_schedule_t weekly_schedule;
    memcpy(&save_data.weekly_schedule, &weekly_schedule, sizeof(weekly_schedule_t));
#endif

    os_time_dly(1); // 先让出cpu，处理其他任务，防止看门狗复位
    syscfg_write(CFG_USER_CLOSE_SEQUENCER_DATA, (u8*)(&save_data), sizeof(save_flash_t));

    flag_is_enable_to_save = 0;
    printf("user data save\n");
    printf("sequencers.on_ff = %u\n", (u16)sequencers.on_ff);
}

// 写入flash时间倒计时
/**
 * @brief 写入flash倒计时
 *      10ms调用一次，不需要特别准确
 *
 *      如果 flag_is_enable_count_down == 1，表示使能倒计时
 *      如果 flag_is_enable_count_down == 0，表示未使能倒计时
 *
 *      计时结束，将 flag_is_enable_to_save 置一
 */
void save_user_data_time_count_down(void)
{
    if (0 == flag_is_enable_count_down)
    {
        return;
    }

    if (time_count_down > 0)
    {
        time_count_down--;
    }

    if (0 == time_count_down)
    {
        flag_is_enable_count_down = 0;
        flag_is_enable_to_save = 1;
    }
}


// 使能保存数据的倒计时，使能保存数据的操作
void save_user_data_enable(void)
{
    flag_is_enable_count_down = 0;
    time_count_down = DELAY_SAVE_FLASH_TIMES / 10; // DELAY_SAVE_FLASH_TIMES / 10 ms计时，实现 DELAY_SAVE_FLASH_TIMES ms延时
    flag_is_enable_count_down = 1;
}

/**
 * @brief 保存用户数据
 *          需要放到主循环执行
 *
 * @return * void
 */
void save_user_data_handle(void)
{
    if (flag_is_enable_to_save)
    {
        flag_is_enable_to_save = 0;
        save_user_data_area3();
    }
}
