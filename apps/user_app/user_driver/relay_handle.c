#include "relay_handle.h"

// 将 继电器 的索引 与 对应的驱动引脚绑定：
// relay_index -> relay_pin_index
const u32 relay_pin_buff[8] = {
    RELAY_PIN_INDEX_0,
    RELAY_PIN_INDEX_1,
    RELAY_PIN_INDEX_2,
    RELAY_PIN_INDEX_3,
    RELAY_PIN_INDEX_4,
    RELAY_PIN_INDEX_5,
    RELAY_PIN_INDEX_6,
    RELAY_PIN_INDEX_7 };

static volatile relay_status_t relay_status_buff[8] = { 0 };

/**
 * @brief 继电器状态初始化，默认为不激活的状态
 *
 */
void relay_status_init(void)
{
    for (int i = 0; i < ARRAY_SIZE(relay_status_buff); i++)
    {
        relay_status_buff[i] = RELAY_STATUS_DEACTIVE;
    }
}


/**
 * @brief 设置继电器的状态，会直接操作继电器
 *
    由于驱动继电器的引脚也会连接到对应按键的led，
    这里也会设置led的状态（点亮或熄灭）
 *
 * @param relay_index 继电器索引，从0开始
 * @param relay_status 继电器状态
 *          RELAY_STATUS_DEACTIVE   继电器 不激活（关闭、断开）
            RELAY_STATUS_ACTIVE   继电器 激活（启动、闭合）
 *
 */
void relay_status_setting(relay_index_t relay_index, relay_status_t relay_status)
{
    if (RELAY_STATUS_ACTIVE == relay_status)
    {
        // 激活继电器 
        gpio_direction_output(relay_pin_buff[relay_index], 1);
    }
    else
    {
        // 不激活继电器 
        gpio_direction_output(relay_pin_buff[relay_index], 0);
    }

    relay_status_buff[relay_index] = relay_status; // 保存继电器状态
}

/**
 * @brief 翻转 继电器的状态 ， 会直接操作继电器
 *
    由于驱动继电器的引脚也会连接到对应按键的led，
    这里也会设置led的状态（点亮或熄灭）
 *
 *
 * @param relay_index 继电器索引，从0开始
 */
void relay_status_toggle(relay_index_t relay_index)
{
    if (RELAY_STATUS_ACTIVE == relay_status_buff[relay_index])
    {
        // 如果对应的继电器是 激活 状态，改成 不激活 状态
        relay_status_setting(relay_index, RELAY_STATUS_DEACTIVE);
    }
    else
    {
        // 如果对应的继电器是 不激活 状态，改成 激活 状态
        relay_status_setting(relay_index, RELAY_STATUS_ACTIVE);
    }
}

/**
 * @brief 延时设置继电器的状态
 *
 *     由于驱动继电器的引脚也会连接到对应按键的led，
        这里也会设置led的状态（点亮或熄灭）
 *

    @param relay_index 继电器索引，从0开始
    @param relay_status 继电器状态
                RELAY_STATUS_DEACTIVE   继电器 不激活（关闭、断开）
                RELAY_STATUS_ACTIVE   继电器 激活（启动、闭合）
    @param delay_ms 延时，单位：10 ms
 */
void relay_status_setting_dly(relay_index_t relay_index, relay_status_t relay_status, u8 delay_time)
{
    os_time_dly(delay_time);
    relay_status_setting(relay_index, relay_status);
}

/**
 * @brief 获取 指定 继电器 的状态
 *
 * @param relay_index 继电器索引
 * @return relay_status_t 继电器状态
 *          RELAY_STATUS_ACTIVE：激活
 *          RELAY_STATUS_DEACTIVE：不激活
 */
relay_status_t relay_status_get(relay_index_t relay_index)
{
    return relay_status_buff[relay_index];
}
