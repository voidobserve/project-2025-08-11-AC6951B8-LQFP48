#ifndef DEVICE_ADKEY_H
#define DEVICE_ADKEY_H

#include "typedef.h"
#include "asm/adc_api.h"
#include "gpio.h"
// #include "app_config.h"





//耀祥时序器相关的灯
//电源指示灯
#define pwoer_light IO_PORTB_00

//LCD屏的指示灯
#define lcd_light IO_PORTA_07

//继电器的灯
#define sw0_led IO_PORTA_12   //总开关
#define sw1_led IO_PORTA_06   //继电器
#define sw2_led IO_PORTA_05
#define sw3_led IO_PORTA_04
#define sw4_led IO_PORTA_03
#define sw5_led IO_PORTA_02
#define sw6_led IO_PORTA_01
#define sw7_led IO_PORTA_00
#define sw8_led IO_PORTC_07


//mp3按键的灯
#define mp3_light_shang IO_PORTA_11
#define mp3_light_zhong IO_PORTC_03
#define mp3_light_xia   IO_PORTC_02


#define ADKEY_MAX_NUM 11   //支持AD按键的数量


struct adkey_platform_data {
    u8 enable;
    u8 adkey_pin;
    u8 extern_up_en;                //是否用外部上拉，1：用外部上拉， 0：用内部上拉10K
    u32 ad_channel;
    u16 ad_value[ADKEY_MAX_NUM];
    u8  key_value[ADKEY_MAX_NUM];
};

struct adkey_rtcvdd_platform_data {
    u8 enable;
    u8 adkey_pin;
    u8  adkey_num;
    u32 ad_channel;
    u32 extern_up_res_value;                //是否用外部上拉，1：用外部上拉， 0：用内部上拉10K
    u16 res_value[ADKEY_MAX_NUM]; 	//电阻值, 从 [大 --> 小] 配置
    u8  key_value[ADKEY_MAX_NUM];
};

//ADKEY API:
extern int adkey_init(const struct adkey_platform_data* adkey_data);
extern int adkey_init2(const struct adkey_platform_data* adkey_data);
extern u8 ad_get_key_value(void);
extern u8 ad_get_key2_value(void);



//RTCVDD ADKEY API:
extern int adkey_rtcvdd_init(const struct adkey_rtcvdd_platform_data* rtcvdd_adkey_data);
extern u8 adkey_rtcvdd_get_key_value(void);

#endif

