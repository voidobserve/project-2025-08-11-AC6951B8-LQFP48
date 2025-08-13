#include "includes.h"
#include "ac_detection.h"



// 滑动平均：
// #define SAMPLE_COUNT 500 // 样本计数
#define SAMPLE_COUNT 1000 // 样本计数
static volatile u16 samples[SAMPLE_COUNT];
static volatile u16 sample_index;

// 更新滑动平均中的数组
void update_filter_samples_buff(u16 adc_val)
{
    u16 i = 0;
    samples[sample_index++] = adc_val;
    if (sample_index >= SAMPLE_COUNT)
        sample_index = 0;
}

// 获取滑动平均后的ad值
u16 get_filter_adc_val(void)
{
    u16 i = 0;
    u32 sum = 0;
    for (i = 0; i < SAMPLE_COUNT; i++)
        sum += samples[i];

    return sum / SAMPLE_COUNT;
}


// void samples_init(u16 adc_val)
// {
//     u16 i = 0;
//     for (i = 0; i < SAMPLE_COUNT; i++)
//     {
//         samples[i] = adc_val;
//     }

// }


void ac_detection_init(void)
{
    // adc_init(); // 在board_init()已经初始化
    adc_add_sample_ch(ADC_CHANNEL_AC_DETECTION); // adc通道
    gpio_set_die(AC_DETECTION_PIN, 0);           // 模拟输入
    gpio_set_direction(AC_DETECTION_PIN, 1);  // 输入
    gpio_set_pull_down(AC_DETECTION_PIN, 0); // 不下拉

    gpio_set_pull_up(AC_DETECTION_PIN, 0); // 不上拉
}


void ac_detection_update(void)
{
    u16 adc_val = 0;
    adc_val = adc_get_value(ADC_CHANNEL_AC_DETECTION);
    update_filter_samples_buff(adc_val);
}


// 更新交流电电压，更新到lcd显示对应的数组中
void ac_voltage_update(void)
{
    static u8 flag_is_first_update = 1;

    u32 ac_voltage = 0;
    u16 adc_filter_val = 0;

    // if (flag_is_first_update)
    // {
    //     flag_is_first_update = 0;
    //     adc_filter_val = adc_get_value(ADC_CHANNEL_AC_DETECTION);
    //     samples_init(adc_filter_val);
    // }

    adc_filter_val = get_filter_adc_val();
    // 交流电压 == 采集到的ad值 / 1024 * 参考电压(单位mV) * 2308（这个是检测脚检测到的电压与实际的交流电压之间的系数，例如 2308，表示 交流电压是检测脚电压的230.8倍） / 10000
    ac_voltage = (u32)adc_filter_val * 3400 * 716 / 1024 / 10000;
    // ac_voltage = (u32)adc_filter_val * 3500 * 2308 / 1024 / 10000;
    extern void get_voltage_array(unsigned long p_v);
    get_voltage_array(ac_voltage);
}
