#include "power_light.h"

void power_light_init(void)
{
    gpio_set_pull_down(POWER_LIGHT_PIN, 0);
    gpio_set_pull_up(POWER_LIGHT_PIN, 0);
    gpio_direction_output(POWER_LIGHT_PIN, 0);
}

void power_light_on(void)
{
    gpio_direction_output(POWER_LIGHT_PIN, 1); // 开灯
}

void power_light_off(void)
{
    gpio_direction_output(POWER_LIGHT_PIN, 0); // 关灯
}


void power_light_toggle(void)
{
    static volatile u8 flag = 0;  //作用：灯闪烁
    if (flag)
        gpio_direction_output(POWER_LIGHT_PIN, 1); // 开灯
    else
        gpio_direction_output(POWER_LIGHT_PIN, 0); //关灯

    flag = !flag;
}


