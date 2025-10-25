#include "includes.h"
#include "ac_detection.h"



// 滑动平均： 
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

// u16 get_max_adc_val_in_samples(void)
// {
//     u16 i = 0;
//     u16 max_adc_val = 0;
//     for (i = 0; i < SAMPLE_COUNT; i++)
//     {
//         if (samples[i] > max_adc_val)
//         {
//             max_adc_val = samples[i];
//         }
//     }

//     return max_adc_val;
// }

// u16 get_min_adc_val_in_samples(void)
// {
//     u16 i = 0;
//     u16 min_adc_val = 0xFFFF;
//     for (i = 0; i < SAMPLE_COUNT; i++)
//     {
//         if (samples[i] < min_adc_val)
//         {
//             min_adc_val = samples[i];
//         }
//     }

//     return min_adc_val;
// }




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

/**
 * @brief
 *
 *  该函数在初始化之后，定时调用
 *
 */
void ac_detection_update(void)
{
#if 1
    u16 adc_val = 0;
    adc_val = adc_get_value(ADC_CHANNEL_AC_DETECTION);
    update_filter_samples_buff(adc_val); // 将数据放入滑动平均数组中
#endif 
}


// 更新交流电电压，更新到lcd显示对应的数组中
/**
 * @brief
 *
 *  程序里是每次取出数组内的一个峰值，再根据对应关系进行计算
 *
 *  该函数在初始化之后，定时调用
 * 
 *      目前每2ms采集一次ad值，每500ms调用一次函数，
 *      那么每500ms更新一次显示
 *
 */
void ac_voltage_update(void)
{
    // lcd显示当前检测到得最低电压值
    u16 adc_val = 0;
    u16 ac_voltage = 0; // 存放得到的交流电电压
    static volatile u16 last_display_voltage = 0;     // 上次显示的交流电电压值
    u16 voltage_diff = 0;                    // 存放当前得到的交流电电压和上次显示的交流电之前的电压差值
    static volatile u8 flag_is_first_update = 1; 

    // adc_val = get_min_adc_val_in_samples();

    // ========================================
    u16 max_adc_val = 0;
    u16 min_adc_val = 0xFFFF;
    u16 diff_between_max_and_min = 0; // 存放 max_adc_val 和 min_adc_val 之间的ad差值
    // ========================================

    for (u16 i = 0; i < SAMPLE_COUNT; i++)
    {
        if (samples[i] < min_adc_val)
        {
            min_adc_val = samples[i];
        }

        if (samples[i] > max_adc_val)
        {
            max_adc_val = samples[i];
        }
    }

    diff_between_max_and_min = max_adc_val - min_adc_val;
    /*
        如果检测到的最大ad值和最小ad值之间的差值小于一定值，
        说明没有交流电输入，可以认为当前检测到的交流电电压为0V

        ad值相差1，说明检测脚检测到的电压相差 2.9296875 mV

        假设差值小于 500mV，认为没有交流电输入，ad值应该相差 170.1
    */
    // if (diff_between_max_and_min < 600)  // 
    if (diff_between_max_and_min < 170)  // 
    {
        ac_voltage = 0;
        adc_val = 0;
    }
    else
    {
        adc_val = min_adc_val; // 得到 samples[] 数组中的最小值
        // 现在显示当前时间段检测到的最低电压，单位：0.01V（参考电压3.0V）
        /*
            250V    检测脚电压最低值约 0.1V
            110V    检测脚电压最低值约 1.01~1.04V

            得到的直线方程：
            x，交流电电压，单位：V
            y，检测脚电压，单位：0.01V
            y = -0.65x + 172.5  （如果是110V交流电，检测脚电压最低值是 1.01V）
            y = -0.67143x + 177.8575 （如果是110V交流电，检测脚电压最低值是 1.04V）

        */
        /*
            将采集到的以0.01V为单位的电压值，转换成以1V为单位的交流电电压值

            根据公式 y = -0.65x + 172.5  （如果是110V交流电，检测较电压最低值是 1.01V）
        */
        // 将ad值转换成单位为0.01V的电压值，再套入公式计算：
        ac_voltage = ((double)adc_val * 3000 / 10 / 1024 - 172.5) / (-0.65);
        // /*
        //     将采集到的以0.01V为单位的电压值，转换成以1V为单位的交流电电压值

        //     根据公式 y = -0.67143x + 177.8575 （如果是110V交流电，检测脚电压最低值是 1.04V）
        // */
        // ac_voltage = ((double)adc_val * 3000 / 10 / 1024 - 177.8575) / (-0.67143);
    }

    if (flag_is_first_update)
    {
        flag_is_first_update = 0;
        // 第一次上电，初始化数组
        ac_voltage_buff_init(adc_val); //  
    }

    ac_voltage_buff_add_new_val(ac_voltage); // 往滑动平均数组添加新的元素
    ac_voltage = ac_voltage_buff_get_filter_val(); // 得到滑动平均滤波后的数值

    if (ac_voltage != 0)
    {
        // 检测到的交流电电压不为0，加上补偿
        ac_voltage += AC_VOLTAGE_DIFF_VALUE;
    }

    // 计算与上次显示值的差值
    if (ac_voltage > last_display_voltage)
    {
        voltage_diff = ac_voltage - last_display_voltage;
    }
    else
    {
        voltage_diff = last_display_voltage - ac_voltage;
    }

    /*
        仅在以下情况更新显示:
        1. 首次更新
        2. 电压变化超过阈值(例如5V)
        3. 电压从有到无或从无到有
    */
#define DISPLAY_UPDATE_THRESHOLD 2
    if (voltage_diff >= DISPLAY_UPDATE_THRESHOLD || /* 当前采集到的电压与之前显示的电压相差过大 */
        (last_display_voltage > 0 && ac_voltage == 0) || /* 电压从有到无 */
        (last_display_voltage == 0 && ac_voltage > 0)) /* 电压从无到有 */
    {

        last_display_voltage = ac_voltage;
    }

    // last_display_voltage = ac_voltage;
    extern void get_voltage_array(unsigned long p_v);
    get_voltage_array(last_display_voltage);
    
    // get_voltage_array(ac_voltage);
}








