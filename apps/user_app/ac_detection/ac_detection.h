#ifndef __AC_DETECTION_H
#define __AC_DETECTION_H

#define AC_DETECTION_PIN IO_PORTB_03
#define ADC_CHANNEL_AC_DETECTION  AD_CH_PB3 // 检测脚对应的adc通道

// 误差修正，检测交流电电压后，会减去这个误差，单位：V
#define AC_VOLTAGE_DIFF_VALUE 4

void ac_detection_init(void);
void ac_detection_update(void);

// 更新交流电电压，更新到lcd显示对应的数组中
void ac_voltage_update(void);

#endif

