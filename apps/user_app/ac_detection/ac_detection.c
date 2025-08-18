#include "includes.h"
#include "ac_detection.h"



// 滑动平均：
// #define SAMPLE_COUNT 500 // 样本计数
// #define SAMPLE_COUNT 1000 // 样本计数
#define SAMPLE_COUNT 2000 // 样本计数
static volatile u16 samples[SAMPLE_COUNT];
static volatile u16 sample_index;


#define AC_VOLTAGE_SIZE 10
static volatile u16 ac_voltage_buff[AC_VOLTAGE_SIZE];
static volatile u16 ac_voltage_buff_index;

// 更新滑动平均中的数组，往数组添加新元素
void update_filter_samples_buff(u16 adc_val)
{
    // u16 i = 0;
    samples[sample_index++] = adc_val;
    if (sample_index >= SAMPLE_COUNT)
        sample_index = 0;
}

// 获取滑动平均后的ad值
// u16 get_filter_adc_val(void)
// {
//     u16 i = 0;
//     u32 sum = 0;
//     for (i = 0; i < SAMPLE_COUNT; i++)
//         sum += samples[i];

//     return sum / SAMPLE_COUNT;
// }

u16 get_max_adc_val_in_samples(void)
{
    u16 i = 0;
    u16 max_adc_val = 0;
    for (i = 0; i < SAMPLE_COUNT; i++)
    {
        if (samples[i] > max_adc_val)
        {
            max_adc_val = samples[i];
        }
    }

    return max_adc_val;
}

 

void ac_voltage_buff_init(u16 ac_voltage)
{
    u16 i = 0;
    for (i = 0; i < AC_VOLTAGE_SIZE; i++)
    {
        ac_voltage_buff[i] = ac_voltage;
    }
}

// 更新滑动平均中的数组
void ac_voltage_buff_add_new_val(u16 ac_voltage)
{
    // u16 i = 0;
    ac_voltage_buff[ac_voltage_buff_index++] = ac_voltage;
    if (ac_voltage_buff_index >= AC_VOLTAGE_SIZE)
        ac_voltage_buff_index = 0;
}

u16 ac_voltage_buff_get_filter_val(void)
{
    u16 i = 0;
    u32 sum = 0;
    for (i = 0; i < AC_VOLTAGE_SIZE; i++)
        sum += ac_voltage_buff[i];

    return sum / AC_VOLTAGE_SIZE;
}



#if 0
// 在滑动平均滤波的基础上修改
// 获取滑动平均数组中1/3的数据作为峰值，再求平均
// static volatile u16 samples_peak[SAMPLE_COUNT / 3];

// 冒泡排序bubble sort
void bubble_sort(u16* array, u16 array_len)
{
    u16 i = 0;
    u16 j = 0;
    u16 temp = 0;
    for (i = 0; i < array_len - 1; i++)
    {
        for (j = i + 1; j < array_len; j++)
        {
            if (array[i] > array[j])
            {
                temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

u16  get_filter_peak_adc_val(void)
{
    u16 i = 0;
    u32 sum = 0;

    bubble_sort(samples, ARRAY_SIZE(samples));

    for (i = 0; i < SAMPLE_COUNT / 3; i++)
    {
        // samples_peak[i] = samples[SAMPLE_COUNT - i];
        sum += samples[SAMPLE_COUNT - i];
    }

    return sum / (SAMPLE_COUNT / 3);
}
#endif

void ac_detection_init(void)
{
    // adc_init(); // 在board_init()已经初始化
    adc_add_sample_ch(ADC_CHANNEL_AC_DETECTION); // adc通道
    gpio_set_die(AC_DETECTION_PIN, 0);           // 模拟输入
    gpio_set_direction(AC_DETECTION_PIN, 1);     // 输入
    gpio_set_pull_down(AC_DETECTION_PIN, 0);     // 不下拉
    gpio_set_pull_up(AC_DETECTION_PIN, 0); // 不上拉
}


void ac_detection_update(void)
{
    u16 adc_val = 0;
    adc_val = adc_get_value(ADC_CHANNEL_AC_DETECTION);
    update_filter_samples_buff(adc_val); // 将数据放入滑动平均数组中
}


// 更新交流电电压，更新到lcd显示对应的数组中
void ac_voltage_update(void)
{
    u32 ac_voltage = 0;
    u16 adc_filter_val = 0;
    static u8 flag_is_first_update = 1;

    adc_filter_val = get_max_adc_val_in_samples();
    ac_voltage =  ((double)adc_filter_val / 1024 * 3 + 0.11) / 0.013;

    if (flag_is_first_update)
    {
        flag_is_first_update = 0;
        // 第一次上电，初始化数组
        ac_voltage_buff_init(ac_voltage);
    }

    ac_voltage_buff_add_new_val(ac_voltage);
    ac_voltage = ac_voltage_buff_get_filter_val();

    // 如果交流电电压小于90V，直接显示0V
    if (ac_voltage < 90)
    {
        ac_voltage = 0;
    }

    // 将采集好的电压值放到LCD显示对应的数组中
    extern void get_voltage_array(unsigned long p_v);
    get_voltage_array(ac_voltage);

#if 0
    // 测试用,让LCD屏显示采集脚检测到的电压，单位：0.1V
    adc_filter_val = adc_filter_val * 3000 / 10 / 1024;
    get_voltage_array(adc_filter_val);
#endif

    /*
        交流电电压，检测脚检测到的最大电压
        220, 2.75
        200, 2.48
        180, 2.21
        160, 1.95
        140, 1.69
        120, 1.43
        100, 1.18
        90, 1.06

        根据这些坐标点绘制图像，得出对应的方程
        y = 0.013𝑥 − 0.11
    */


}
