#ifndef BOARD_AC695X_MULTIMEDIA_CHARGING_BIN_H
#define BOARD_AC695X_MULTIMEDIA_CHARGING_BIN_H

#ifdef CONFIG_BOARD_AC695X_MULTIMEDIA_CHARGING_BIN

#define CONFIG_SDFILE_ENABLE
#define CONFIG_FLASH_SIZE       (1024 * 1024)

//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define LINEIN_INPUT_WAY_ANALOG      0
#define LINEIN_INPUT_WAY_ADC         1

#define NO_CONFIG_PORT						(-1)

//*********************************************************************************//
//                                  app 配置                                       //
//*********************************************************************************//
#define TCFG_APP_BT_EN			            1
#define TCFG_APP_MUSIC_EN			        1
#define TCFG_APP_LINEIN_EN					1
#define TCFG_APP_FM_EN					    0
#define TCFG_APP_PC_EN					    1
#define TCFG_APP_RTC_EN					    1
#define TCFG_APP_RECORD_EN				    1
#define TCFG_APP_SPDIF_EN                   0

//*********************************************************************************//
//                               PCM_DEBUG调试配置                                 //
//*********************************************************************************//

//#define AUDIO_PCM_DEBUG					  	//PCM串口调试，写卡通话数据

//*********************************************************************************//
//                                 UART配置                                        //
//*********************************************************************************//
#define TCFG_UART0_ENABLE					ENABLE_THIS_MOUDLE                     //串口打印模块使能
#define TCFG_UART0_RX_PORT					NO_CONFIG_PORT                         //串口接收脚配置（用于打印可以选择NO_CONFIG_PORT）
#define TCFG_UART0_TX_PORT  				IO_PORT_DP                            //串口发送脚配置
#define TCFG_UART0_BAUDRATE  				1000000                                //串口波特率配置

//*********************************************************************************//
//                                 IIC配置                                        //
//*********************************************************************************//
/*软件IIC设置*/
#define TCFG_SW_I2C0_CLK_PORT               IO_PORTA_09                             //软件IIC  CLK脚选择
#define TCFG_SW_I2C0_DAT_PORT               IO_PORTA_10                             //软件IIC  DAT脚选择
#define TCFG_SW_I2C0_DELAY_CNT              50                                      //IIC延时参数，影响通讯时钟频率

//A组IO: SDA:  DM    SCL:  DP           B组IO: SDA: PC4    SCL: PC5
//C组IO: SDA: PB4    SCL: PB6           D组IO: SDA: PA5    SCL: PA6
#define TCFG_HW_I2C0_PORTS                  'B'                                     //选择第几组硬件引脚
#define TCFG_HW_I2C0_CLK                    100000                                  //硬件IIC波特率

//*********************************************************************************//
//                                 硬件SPI 配置                                        //
//*********************************************************************************//
#define	TCFG_HW_SPI1_ENABLE		ENABLE_THIS_MOUDLE
//A组IO:    DI: PB2     DO: PB1     CLK: PB0
//B组IO:    DI: PC3     DO: PC5     CLK: PC4
#define TCFG_HW_SPI1_PORT		'B'
#define TCFG_HW_SPI1_BAUD		20000000L
#define TCFG_HW_SPI1_MODE		SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI1_ROLE		SPI_ROLE_MASTER

#define	TCFG_HW_SPI2_ENABLE		ENABLE_THIS_MOUDLE
//A组IO:    DI: PB8     DO: PB10    CLK: PB9
//B组IO:    DI: PA13    DO: DM      CLK: DP
#define TCFG_HW_SPI2_PORT		'A'
#define TCFG_HW_SPI2_BAUD		2000000L
#define TCFG_HW_SPI2_MODE		SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI2_ROLE		SPI_ROLE_MASTER

//*********************************************************************************//
//                                 FLASH 配置                                      //
//*********************************************************************************//
#define TCFG_NORFLASH_DEV_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_FLASH_DEV_SPI_HW_NUM			1// 1: SPI1    2: SPI2
#define TCFG_FLASH_DEV_SPI_CS_PORT	    	IO_PORTA_03


//*********************************************************************************//
//                                  充电参数配置                                   //
//*********************************************************************************//
//是否支持芯片内置充电
#define TCFG_CHARGE_ENABLE					DISABLE_THIS_MOUDLE
//是否支持开机充电
#define TCFG_CHARGE_POWERON_ENABLE			DISABLE
//是否支持拔出充电自动开机功能
#define TCFG_CHARGE_OFF_POWERON_NE			DISABLE

#define TCFG_CHARGE_FULL_V					CHARGE_FULL_V_4202

#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_10

#define TCFG_CHARGE_MA						CHARGE_mA_60


//*********************************************************************************//
//                                  SD 配置                                        //
//*********************************************************************************//
#define TCFG_SD0_ENABLE						ENABLE_THIS_MOUDLE
//A组IO: CMD:PA9    CLK:PA10   DAT0:PA5    DAT1:PA6    DAT2:PA7    DAT3:PA8
//B组IO: CMD:PB10   CLK:PB9    DAT0:PB8    DAT1:PB6    DAT2:PB5    DAT3:PB4
#define TCFG_SD0_PORTS						'B'
#define TCFG_SD0_DAT_MODE					1
#define TCFG_SD0_DET_MODE					SD_CLK_DECT
#define TCFG_SD0_DET_IO 					IO_PORT_DM//当SD_DET_MODE为2时有效
#define TCFG_SD0_DET_IO_LEVEL				0//IO检查，0：低电平检测到卡。 1：高电平(外部电源)检测到卡。 2：高电平(SD卡电源)检测到卡。
#define TCFG_SD0_CLK						(3000000*4L)

#define TCFG_SD1_ENABLE						DISABLE_THIS_MOUDLE
//A组IO: CMD:PC4    CLK:PC5    DAT0:PC3    DAT1:PC2    DAT2:PC1    DAT3:PC0
//B组IO: CMD:PB5    CLK:PB6    DAT0:PB4    DAT1:PB8    DAT2:PB9    DAT3:PB10
#define TCFG_SD1_PORTS						'A'
#define TCFG_SD1_DAT_MODE					1
#define TCFG_SD1_DET_MODE					SD_CLK_DECT
#define TCFG_SD1_DET_IO 					IO_PORT_DM//当SD_DET_MODE为2时有效
#define TCFG_SD1_DET_IO_LEVEL				0//IO检查，0：低电平检测到卡。 1：高电平(外部电源)检测到卡。 2：高电平(SD卡电源)检测到卡。
#define TCFG_SD1_CLK						(3000000*4L)


//*********************************************************************************//
//                                 USB 配置                                        //
//*********************************************************************************//
#define TCFG_PC_ENABLE						ENABLE_THIS_MOUDLE//PC模块使能
#define TCFG_UDISK_ENABLE					DISABLE_THIS_MOUDLE//U盘模块使能
#define TCFG_OTG_USB_DEV_EN                 BIT(0)//USB0 = BIT(0)  USB1 = BIT(1)

#include "usb_std_class_def.h"


//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
//#define KEY_NUM_MAX                        	10
//#define KEY_NUM                            	3
#define KEY_IO_NUM_MAX						6
#define KEY_AD_NUM_MAX						10
#define KEY_IR_NUM_MAX						21
#define KEY_TOUCH_NUM_MAX					6
#define KEY_RDEC_NUM_MAX                    3
#define KEY_CTMU_TOUCH_NUM_MAX				6

#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表
//*********************************************************************************//
//                                 iokey 配置                                      //
//*********************************************************************************//
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能IO按键

#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO

#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTB_01        //IO按键端口

#define TCFG_IOKEY_PREV_CONNECT_WAY			ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_PREV_ONE_PORT			IO_PORTB_00

#define TCFG_IOKEY_NEXT_CONNECT_WAY 		ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_NEXT_ONE_PORT			IO_PORTB_02

//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define TCFG_ADKEY_ENABLE                   ENABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_ADKEY_PORT                     IO_PORTB_01         //AD按键端口(需要注意选择的IO口是否支持AD功能)
#define TCFG_ADKEY_AD_CHANNEL               AD_CH_PB1
#define TCFG_ADKEY_EXTERN_UP_ENABLE         ENABLE_THIS_MOUDLE //是否使用外部上拉

#if TCFG_ADKEY_EXTERN_UP_ENABLE
#define R_UP    220                 //22K，外部上拉阻值在此自行设置
#else
#define R_UP    100                 //10K，内部上拉默认10K
#endif

//必须从小到大填电阻，没有则同VDDIO,填0x3ffL
#define TCFG_ADKEY_AD0      (0)                                 //0R
#define TCFG_ADKEY_AD1      (0x3ffL * 51   / (51   + R_UP))     //3k
#define TCFG_ADKEY_AD2      (0x3ffL * 150  / (150  + R_UP))     //6.2k
#define TCFG_ADKEY_AD3      (0x3ffL * 330  / (330  + R_UP))     //9.1k
#define TCFG_ADKEY_AD4      (0x3ffL * 1000 / (1000 + R_UP))     //15k
#define TCFG_ADKEY_AD5      (0x3ffL * 2200 / (2200  + R_UP))     //24k
#define TCFG_ADKEY_AD6      (0x3ffL)// * 330  / (330  + R_UP))     //33k
#define TCFG_ADKEY_AD7      (0x3ffL)// * 510  / (510  + R_UP))     //51k
#define TCFG_ADKEY_AD8      (0x3ffL)//* 1000 / (1000 + R_UP))     //100k
#define TCFG_ADKEY_AD9      (0x3ffL)// * 2200 / (2200 + R_UP))     //220k
#define TCFG_ADKEY_VDDIO    (0x3ffL)

#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   4
#define TCFG_ADKEY_VALUE5                   5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   9

//*********************************************************************************//
//                                 irkey 配置                                      //
//*********************************************************************************//
#define TCFG_IRKEY_ENABLE                   DISABLE_THIS_MOUDLE//是否使能ir按键
#define TCFG_IRKEY_PORT                     IO_PORTA_02        //IR按键端口

//*********************************************************************************//
//                            ctmu tocuh key 配置                                      //
//*********************************************************************************//
#define TCFG_CTMU_TOUCH_KEY_ENABLE              DISABLE_THIS_MOUDLE             //是否使能CTMU触摸按键
#define TCFG_CTMU_TOUCH_KEY_PRESS_CFG 		   	40//按下灵敏度（s16）,数值越小, 灵敏度越高，一般设置30-100
#define TCFG_CTMU_TOUCH_KEY_RELEASE_CFG0 		10 //释放灵敏度0（s16），数值越小，灵敏度越高，必须比按下灵敏度小
#define TCFG_CTMU_TOUCH_KEY_RELEASE_CFG1 		160 //释放灵敏度1（s16）, 数值越小, 灵敏度越高，一般只需要调节上面两个

//key0配置
#define TCFG_CTMU_TOUCH_KEY0_PORT 				IO_PORTA_00  //触摸按键key0 IO配置
#define TCFG_CTMU_TOUCH_KEY0_VALUE 				0 		 	 //触摸按键key0 按键值

//key1配置
#define TCFG_CTMU_TOUCH_KEY1_PORT 				IO_PORTA_01  //触摸按键key1 IO配置
#define TCFG_CTMU_TOUCH_KEY1_VALUE 				1 		 	 //触摸按键key1 按键值

//*********************************************************************************//
//                                 rdec_key 配置                                      //
//*********************************************************************************//
#define TCFG_RDEC_KEY_ENABLE				    DISABLE_THIS_MOUDLE //是否使能RDEC按键
//默认硬件引脚配置
//RDEC0配置
#define TCFG_RDEC0_ECODE1_PORT					IO_PORTA_02
#define TCFG_RDEC0_ECODE2_PORT					IO_PORTA_03
#define TCFG_RDEC0_KEY0_VALUE 				 	0
#define TCFG_RDEC0_KEY1_VALUE 				 	1

//RDEC1配置
#define TCFG_RDEC1_ECODE1_PORT					IO_PORTB_02
#define TCFG_RDEC1_ECODE2_PORT					IO_PORTB_03
#define TCFG_RDEC1_KEY0_VALUE 				 	2
#define TCFG_RDEC1_KEY1_VALUE 				 	3

//RDEC2配置
#define TCFG_RDEC2_ECODE1_PORT					IO_PORTB_04
#define TCFG_RDEC2_ECODE2_PORT					IO_PORTB_06
#define TCFG_RDEC2_KEY0_VALUE 				 	4
#define TCFG_RDEC2_KEY1_VALUE 				 	5


//*********************************************************************************//
//                                 Audio配置                                       //
//*********************************************************************************//
#define TCFG_AUDIO_ADC_ENABLE				ENABLE_THIS_MOUDLE
//MIC只有一个声道，固定选择右声道
#define TCFG_AUDIO_ADC_MIC_CHA				LADC_CH_MIC_R
//省电容MIC使能
#define TCFG_MIC_CAPLESS_ENABLE				DISABLE_THIS_MOUDLE
/*MIC LDO电流档位设置：
    0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
#define TCFG_AUDIO_ADC_LDO_SEL				2

// LADC通道
#define TCFG_AUDIO_ADC_LINE_CHA0			LADC_LINE1_MASK
#define TCFG_AUDIO_ADC_LINE_CHA1			LADC_CH_LINE0_L

#define TCFG_AUDIO_DAC_ENABLE				ENABLE_THIS_MOUDLE
#define TCFG_AUDIO_DAC_LDO_SEL				1

#define TCFG_AUDIO_DAC_LDO_VOLT				DACVDD_LDO_2_90V
/*预留接口，未使用*/
#define TCFG_AUDIO_DAC_PA_PORT				NO_CONFIG_PORT
/*
DAC硬件上的连接方式,可选的配置：
DAC_OUTPUT_LR                  立体声
DAC_OUTPUT_FRONT_LR_REAR_LR    四声道输出
*/
// #define TCFG_AUDIO_DAC_CONNECT_MODE    DAC_OUTPUT_FRONT_LR_REAR_LR
#define TCFG_AUDIO_DAC_CONNECT_MODE    DAC_OUTPUT_LR

/*通话降噪模式配置*/
#define CVP_ANS_MODE	0	/*传统降噪*/
#define CVP_DNS_MODE	1	/*神经网络降噪*/
#define TCFG_AUDIO_CVP_NS_MODE				CVP_ANS_MODE

#define AUDIO_OUTPUT_WAY_DAC        0
#define AUDIO_OUTPUT_WAY_IIS        1
#define AUDIO_OUTPUT_WAY_FM         2
#define AUDIO_OUTPUT_WAY_HDMI       3
#define AUDIO_OUTPUT_WAY_SPDIF      4
#define AUDIO_OUTPUT_WAY_BT      	5	// bt emitter
#define AUDIO_OUTPUT_WAY_DAC_IIS    6
#define AUDIO_OUTPUT_WAY_DONGLE		7
#define AUDIO_OUTPUT_WAY            AUDIO_OUTPUT_WAY_DAC
#define LINEIN_INPUT_WAY            LINEIN_INPUT_WAY_ANALOG

#define AUDIO_OUTPUT_AUTOMUTE       0//ENABLE

// 每个解码通道都开启数字音量管理
#define SYS_DIGVOL_GROUP_EN         DISABLE

/*
 *系统音量类型选择
 *软件数字音量是指纯软件对声音进行运算后得到的
 *硬件数字音量是指dac内部数字模块对声音进行运算后输出
 */
#define VOL_TYPE_DIGITAL		0	//软件数字音量
#define VOL_TYPE_ANALOG			1	//硬件模拟音量
#define VOL_TYPE_AD				2	//联合音量(模拟数字混合调节)
#define VOL_TYPE_DIGITAL_HW		3  	//硬件数字音量
#define SYS_VOL_TYPE            VOL_TYPE_ANALOG
/*
 *通话的时候使用数字音量
 *0：通话使用和SYS_VOL_TYPE一样的音量调节类型
 *1：通话使用数字音量调节，更加平滑
 */
#define TCFG_CALL_USE_DIGITAL_VOLUME		0

// 使能改宏，提示音音量使用music音量
#define APP_AUDIO_STATE_WTONE_BY_MUSIC      (1)
// 0:提示音不使用默认音量； 1:默认提示音音量值
#define TONE_MODE_DEFAULE_VOLUME            (0)
//*********************************************************************************//
//                                  充电仓配置                                     //
//*********************************************************************************//
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE       //是否支持智能充点仓
#define TCFG_TEST_BOX_ENABLE			    0
#define TCFG_CHARGESTORE_PORT				IO_PORTA_02               //耳机和充点仓通讯的IO口
#define TCFG_CHARGESTORE_UART_ID			IRQ_UART1_IDX             //通讯使用的串口号

#ifdef AUDIO_PCM_DEBUG
#ifdef	TCFG_TEST_BOX_ENABLE
#undef 	TCFG_TEST_BOX_ENABLE
#define TCFG_TEST_BOX_ENABLE				0		//因为使用PCM使用到了串口1
#endif
#endif/*AUDIO_PCM_DEBUG*/

//*********************************************************************************//
//                                  LED 配置                                       //
//*********************************************************************************//
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE	//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE		//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_06			//LED使用的IO口

//*********************************************************************************//
//                                  UI 配置                                        //
//*********************************************************************************//
#define TCFG_UI_ENABLE 						ENABLE_THIS_MOUDLE 	//UI总开关
#define CONFIG_UI_STYLE                     STYLE_JL_SOUNDBOX
// #define TCFG_UI_LED7_ENABLE 			 	ENABLE_THIS_MOUDLE 	//UI使用LED7显示
// #define TCFG_UI_LCD_SEG3X9_ENABLE 		ENABLE_THIS_MOUDLE 	//UI使用LCD段码屏显示
// #define TCFG_LCD_ST7735S_ENABLE	        ENABLE_THIS_MOUDLE
// #define TCFG_LCD_ST7789V_ENABLE          DISABLE_THIS_MOUDLE
// #define TCFG_LCD_ST7789VW_ENABLE	        DISABLE_THIS_MOUDLE
#define TCFG_LCD_OLED_ENABLE	            ENABLE_THIS_MOUDLE
#define TCFG_LRC_LYRICS_ENABLE              ENABLE_THIS_MOUDLE  //歌词显示
#define TCFG_SPI_LCD_ENABLE                 ENABLE_THIS_MOUDLE //spi lcd开关
#define TCFG_TFT_LCD_DEV_SPI_HW_NUM			 1// 1: SPI1    2: SPI2 配置lcd选择的spi口

#define TCFG_LED7_RUN_RAM 					DISABLE_THIS_MOUDLE 	//led7跑ram 不屏蔽中断(需要占据2k附近ram)
//*********************************************************************************//
//                                  时钟配置                                       //
//*********************************************************************************//
#define TCFG_CLOCK_SYS_SRC					SYS_CLOCK_INPUT_PLL_BT_OSC   //系统时钟源选择
#define TCFG_CLOCK_SYS_HZ					24000000                     //系统时钟设置
#define TCFG_CLOCK_OSC_HZ					24000000                     //外界晶振频率设置
#define TCFG_CLOCK_MODE                     CLOCK_MODE_ADAPTIVE

//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15                    //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                            //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL		0//	SLEEP_EN                     //SNIFF状态下芯片是否进入powerdown


#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_34V

#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_28V               //弱VDDIO等级配置


//*********************************************************************************//
//                                  EQ配置                                         //
//*********************************************************************************//
//EQ配置，使用在线EQ时，EQ文件和EQ模式无效。有EQ文件时，使能TCFG_USE_EQ_FILE,默认不用EQ模式切换功能
#define TCFG_EQ_ENABLE                      0     //支持EQ功能,EQ总使能
#if TCFG_EQ_ENABLE
#define TCFG_EQ_ONLINE_ENABLE               0     //支持在线EQ调试
#define TCFG_BT_MUSIC_EQ_ENABLE             0     //支持蓝牙音乐EQ
#define TCFG_PHONE_EQ_ENABLE                0     //支持通话近端EQ
#define TCFG_MUSIC_MODE_EQ_ENABLE           0     //支持音乐模式EQ
#define TCFG_LINEIN_MODE_EQ_ENABLE          0     //支持linein近端EQ
#define TCFG_FM_MODE_EQ_ENABLE              0     //支持fm模式EQ
#define TCFG_SPDIF_MODE_EQ_ENABLE           0     //支持SPDIF模式EQ
#define TCFG_PC_MODE_EQ_ENABLE              0     //支持pc模式EQ
#define TCFG_AUDIO_OUT_EQ_ENABLE			0 	  //mix_out后高低音EQ


#define TCFG_USE_EQ_FILE                    0    //离线eq使用配置文件还是默认系数表 1：使用文件  0 使用默认系数表
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
#define TCFG_EQ_DIVIDE_ENABLE               0     // 四声道eq是否独立  0 使用同个eq效果
#endif

#define EQ_SECTION_MAX                      20//eq 段数
#endif//TCFG_EQ_ENABLE

#define TCFG_DRC_ENABLE						0 	  //DRC 总使能
#if TCFG_DRC_ENABLE
#define TCFG_BT_MUSIC_DRC_ENABLE            0     //支持蓝牙音乐DRC
#define TCFG_MUSIC_MODE_DRC_ENABLE          0     //支持音乐模式DRC
#define TCFG_LINEIN_MODE_DRC_ENABLE         0     //支持LINEIN模式DRC
#define TCFG_FM_MODE_DRC_ENABLE             0     //支持FM模式DRC
#define TCFG_SPDIF_MODE_DRC_ENABLE          0     //支持SPDIF模式DRC
#define TCFG_PC_MODE_DRC_ENABLE             0     //支持PC模式DRC
#define TCFG_AUDIO_OUT_DRC_ENABLE			0 	  //mix_out后drc
#endif//TCFG_DRC_ENABLE




// ONLINE CCONFIG
// 使用eq调试串口时，需关闭usb宏
#define TCFG_ONLINE_ENABLE                  (TCFG_EQ_ONLINE_ENABLE)    //是否支持EQ在线调试功能
#define TCFG_ONLINE_TX_PORT					IO_PORT_DP                 //EQ调试TX口选择
#define TCFG_ONLINE_RX_PORT					IO_PORT_DM                 //EQ调试RX口选择
/***********************************非用户配置区***********************************/
#if TCFG_EQ_ONLINE_ENABLE
#if (TCFG_USE_EQ_FILE == 0)
#undef TCFG_USE_EQ_FILE
#define TCFG_USE_EQ_FILE                    1    //开在线调试时，打开使用离线配置文件宏定义
#endif
#if TCFG_AUDIO_OUT_EQ_ENABLE
#undef TCFG_AUDIO_OUT_EQ_ENABLE
#define TCFG_AUDIO_OUT_EQ_ENABLE            0    //开在线调试时，关闭高低音
#endif
#endif
/**********************************************************************************/

//*********************************************************************************//
//                                  混响配置                                   //
//*********************************************************************************//
#define TCFG_REVERB_ENABLE                  DISABLE
#if (TCFG_REVERB_ENABLE)
#define TCFG_REVERB_HOWLING_EN              DISABLE
#define TCFG_REVERB_PITCH_EN                DISABLE

#if TCFG_EQ_ENABLE
#define TCFG_REVERB_EQ_EN                   1     //混响高低音使能
#else
#define TCFG_REVERB_EQ_EN                   0
#endif

#define TCFG_REVERB_DODGE_EN                0//闪避

#define TCFG_REVERB_SAMPLERATE_DEFUALT	    (44100L) ///混响默认采样率配置

#else
#define TCFG_REVERB_HOWLING_EN              DISABLE//不开混响不支持
#define TCFG_REVERB_PITCH_EN                DISABLE//不开混响不支持
#define TCFG_REVERB_EQ_EN                   0     //混响高低音B使能
#endif//TCFG_REVERB_ENABLE

//*********************************************************************************//
//                                  g-sensor配置                                   //
//*********************************************************************************//
#define TCFG_GSENSOR_ENABLE                       0     //gSensor使能
#define TCFG_DA230_EN                             0
#define TCFG_SC7A20_EN                            0
#define TCFG_STK8321_EN                           0
#define TCFG_GSENOR_USER_IIC_TYPE                 0     //0:软件IIC  1:硬件IIC

//*********************************************************************************//
//                                  系统配置                                         //
//*********************************************************************************//
#define TCFG_AUTO_SHUT_DOWN_TIME		    0   //没有蓝牙连接自动关机时间
#define TCFG_SYS_LVD_EN						1   //电量检测使能
#define TCFG_POWER_ON_NEED_KEY				0	  //是否需要按按键开机配置
#define TWFG_APP_POWERON_IGNORE_DEV         3700//上电忽略挂载设备，0时不忽略，非0则n毫秒忽略

//*********************************************************************************//
//                                  蓝牙配置                                       //
//*********************************************************************************//
#define TCFG_USER_TWS_ENABLE                0   //tws功能使能
#define TCFG_USER_BLE_ENABLE                0   //BLE功能使能
#define TCFG_USER_BT_CLASSIC_ENABLE         1   //经典蓝牙功能使能
#define TCFG_BT_SUPPORT_AAC                 0   //AAC格式支持
#define TCFG_USER_EMITTER_ENABLE            0   //emitter功能使能
#define TCFG_BT_SNIFF_ENABLE                0   //bt sniff 功能使能

#define USER_SUPPORT_PROFILE_SPP    0
#define USER_SUPPORT_PROFILE_HFP    1
#define USER_SUPPORT_PROFILE_A2DP   1
#define USER_SUPPORT_PROFILE_AVCTP  1
#define USER_SUPPORT_PROFILE_HID    1
#define USER_SUPPORT_PROFILE_PNP    1
#define USER_SUPPORT_PROFILE_PBAP   0

#define USER_SUPPORT_DUAL_A2DP_SOURCE              0

#if TCFG_USER_TWS_ENABLE
#define TCFG_BD_NUM						    1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0   //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           0   //对箱通话slave出声音
#else
#define TCFG_BD_NUM						    1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0 //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           0   //对箱通话slave出声音
#endif

#define BT_INBAND_RINGTONE                  0   //是否播放手机自带来电铃声
#define BT_PHONE_NUMBER                     1   //是否播放来电报号
#define BT_SUPPORT_DISPLAY_BAT              1   //是否使能电量检测
#define BT_SUPPORT_MUSIC_VOL_SYNC           1   //是否使能音量同步

#define TCFG_BLUETOOTH_BACK_MODE			1	//后台模式

#if (TCFG_USER_TWS_ENABLE && TCFG_BLUETOOTH_BACK_MODE) && (TCFG_BT_SNIFF_ENABLE==0) && defined(CONFIG_LOCAL_TWS_ENABLE)
#define TCFG_DEC2TWS_ENABLE					1	// 本地解码转发
#define TCFG_PCM_ENC2TWS_ENABLE				1	// pcm编码转发
#define TCFG_PCM2TWS_SBC_ENABLE				1	// pcm转发采样sbc编码
#define TCFG_TONE2TWS_ENABLE				1	// 提示音转发
#else
#define TCFG_DEC2TWS_ENABLE					0
#define TCFG_PCM_ENC2TWS_ENABLE				0
#define TCFG_PCM2TWS_SBC_ENABLE				0
#define TCFG_TONE2TWS_ENABLE				0
#endif

//*********************************************************************************//
//                                  linein配置                                     //
//*********************************************************************************//
#define TCFG_LINEIN_ENABLE					TCFG_APP_LINEIN_EN	// linein使能
// #define TCFG_LINEIN_LADC_IDX				0					// linein使用的ladc通道，对应ladc_list
#define TCFG_LINEIN_LR_CH					AUDIO_LIN1R_CH
#define TCFG_LINEIN_CHECK_PORT				IO_PORTB_10			// linein检测IO
#define TCFG_LINEIN_PORT_UP_ENABLE        	1					// 检测IO上拉使能
#define TCFG_LINEIN_PORT_DOWN_ENABLE       	0					// 检测IO下拉使能
#define TCFG_LINEIN_AD_CHANNEL             	AD_CH_PB10		// 检测IO是否使用AD检测
#define TCFG_LINEIN_VOLTAGE                	1000					// AD检测时的阀值
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG//LINEIN_INPUT_WAY_ADC//
#define TCFG_LINEIN_MULTIPLEX_WITH_SD		0//ENABLE 				// linein 检测与 SD cmd 复用

//*********************************************************************************//
//                                  music 配置                                     //
//*********************************************************************************//
#define TCFG_DEC_G729_ENABLE                ENABLE
#define TCFG_DEC_MP3_ENABLE					ENABLE
#define TCFG_DEC_WMA_ENABLE					ENABLE
#define TCFG_DEC_WAV_ENABLE					ENABLE
#define TCFG_DEC_FLAC_ENABLE				DISABLE
#define TCFG_DEC_APE_ENABLE					DISABLE
#define TCFG_DEC_M4A_ENABLE					ENABLE
#define TCFG_DEC_ALAC_ENABLE				DISABLE
#define TCFG_DEC_AMR_ENABLE					ENABLE
#define TCFG_DEC_DTS_ENABLE					DISABLE
#define TCFG_DEC_G726_ENABLE			    DISABLE
#define TCFG_DEC_MIDI_ENABLE			    DISABLE
#define TCFG_DEC_MTY_ENABLE					DISABLE
#define TCFG_DEC_SBC_ENABLE					ENABLE
#define TCFG_DEC_PCM_ENABLE					ENABLE
#define TCFG_DEC_CVSD_ENABLE				ENABLE
#define TCFG_DEC_WTGV2_ENABLE				DISABLE

#define TCFG_DEC_ID3_V1_ENABLE				DISABLE
#define TCFG_DEC_ID3_V2_ENABLE				DISABLE
#define TCFG_DEC_DECRYPT_ENABLE				DISABLE
#define TCFG_DEC_DECRYPT_KEY				(0x12345678)

////<变速变调
#define TCFG_SPEED_PITCH_ENABLE             DISABLE//
//*********************************************************************************//
//                                  fm 配置                                     //
//*********************************************************************************//
#define TCFG_FM_ENABLE							TCFG_APP_FM_EN // fm 使能
#define TCFG_FM_INSIDE_ENABLE					DISABLE
#define TCFG_FM_RDA5807_ENABLE					DISABLE
#define TCFG_FM_BK1080_ENABLE					DISABLE
#define TCFG_FM_QN8035_ENABLE					DISABLE

#define TCFG_FMIN_LADC_IDX				1				// linein使用的ladc通道，对应ladc_list
#define TCFG_FMIN_LR_CH					AUDIO_LIN1_LR
#define TCFG_FM_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG

#if (TCFG_FM_INSIDE_ENABLE && TCFG_FM_ENABLE)
#if (((TCFG_USER_TWS_ENABLE) || (RECORDER_MIX_EN) || (TCFG_MIC_EFFECT_ENABLE)))
#define TCFG_CODE_RUN_RAM_FM_MODE 					DISABLE_THIS_MOUDLE  	//FM模式 代码跑ram
#else
#define TCFG_CODE_RUN_RAM_FM_MODE 					ENABLE_THIS_MOUDLE  	//FM模式 代码跑ram
#endif
#else
#define TCFG_CODE_RUN_RAM_FM_MODE 					DISABLE_THIS_MOUDLE 	//FM模式 代码跑ram
#endif /*(TCFG_FM_INSIDE_ENABLE && TCFG_FM_ENABLE)*/

#if (TCFG_CODE_RUN_RAM_FM_MODE && TCFG_UI_ENABLE)
#undef TCFG_LED7_RUN_RAM
#define TCFG_LED7_RUN_RAM 					ENABLE_THIS_MOUDLE 	//led7跑ram 不屏蔽中断(需要占据2k附近ram)
#endif /*(TCFG_CODE_RUN_RAM_FM_MODE && TCFG_UI_ENABLE)*/

//*********************************************************************************//
//                                  fm emitter 配置                                     //
//*********************************************************************************//
#define TCFG_APP_FM_EMITTER_EN                  DISABLE_THIS_MOUDLE
#define TCFG_FM_EMITTER_INSIDE_ENABLE			DISABLE
#define TCFG_FM_EMITTER_AC3433_ENABLE			DISABLE
#define TCFG_FM_EMITTER_QN8007_ENABLE			DISABLE
#define TCFG_FM_EMITTER_QN8027_ENABLE			DISABLE

//*********************************************************************************//
//                                  rtc 配置                                     //
//*********************************************************************************//
#define TCFG_RTC_ENABLE						TCFG_APP_RTC_EN

#define TCFG_USE_VIRTUAL_RTC                   0//ENABLE    //假时钟


//*********************************************************************************//
//                                  SPDIF & ARC 配置                                     //
//*********************************************************************************//
#define TCFG_SPDIF_ENABLE                       TCFG_APP_SPDIF_EN
#define TCFG_SPDIF_OUTPUT_ENABLE                ENABLE
#define TCFG_HDMI_ARC_ENABLE                    ENABLE
#define TCFG_HDMI_CEC_PORT                      IO_PORTA_02

//*********************************************************************************//
//                                  IIS 配置                                     //
//*********************************************************************************//
#define TCFG_IIS_ENABLE                       DISABLE_THIS_MOUDLE //
#define TCFG_IIS_OUTPUT_EN                    ENABLE //
#define TCFG_IIS_OUTPUT_PORT                  ALINK1_PORTA
#define TCFG_IIS_OUTPUT_CH_NUM                1 //0:mono,1:stereo
#define TCFG_IIS_OUTPUT_SR                    44100
#define TCFG_IIS_OUTPUT_DATAPORT_SEL          (BIT(0))

#define TCFG_IIS_INPUT_EN                    DISABLE//
#define TCFG_IIS_INPUT_PORT                  ALINK1_PORTA
#define TCFG_IIS_INPUT_CH_NUM                1 //0:mono,1:stereo
#define TCFG_IIS_INPUT_SR                    (44100l)
#define TCFG_IIS_INPUT_DATAPORT_SEL          (BIT(0))

//*********************************************************************************//
//                                  fat 文件系统配置                                       //
//*********************************************************************************//
#define CONFIG_FATFS_ENABLE					ENABLE
#define TCFG_DEC_FILE_NAME_ENABLE			1//DISABLE//支持长文件名

//*********************************************************************************//
//                                  app 配置                                       //
//*********************************************************************************//

#define TCFG_APP_MUSIC_EN					((TCFG_UDISK_ENALBE || TCFG_SD0_ENABLE || TCFG_SD1_ENABLE) && CONFIG_FATFS_ENABLE)
#define TCFG_APP_LINEIN_EN					(TCFG_LINEIN_ENABLE)
#define TCFG_APP_FM_EN					    (TCFG_FM_ENABLE)
#define TCFG_APP_PC_EN					    (TCFG_PC_ENABLE)
#define TCFG_APP_RTC_EN					    (TCFG_RTC_ENABLE)
#define TCFG_APP_RECORD_EN				    DISABLE_THIS_MOUDLE//((TCFG_UDISK_ENABLE || TCFG_SD0_ENABLE || TCFG_SD1_ENABLE) && CONFIG_FATFS_ENABLE)
#define TCFG_APP_SPDIF_EN                   (TCFG_SPDIF_ENABLE)


//*********************************************************************************//
//                                  REC 配置                                       //
//*********************************************************************************//
#define TCFG_LINEIN_REC_EN					DISABLE
#define	TCFG_MIXERCH_REC_EN			    	DISABLE
#define TCFG_MIC_REC_PITCH_EN               DISABLE
#define TCFG_MIC_REC_REVERB_EN              DISABLE

//*********************************************************************************//
//                                  encoder 配置                                   //
//*********************************************************************************//
#define TCFG_ENC_CVSD_ENABLE               ENABLE
#define TCFG_ENC_MSBC_ENABLE               ENABLE
#define TCFG_ENC_G726_ENABLE               ENABLE
#define TCFG_ENC_MP3_ENABLE                ENABLE
#define TCFG_ENC_ADPCM_ENABLE              ENABLE
#define TCFG_ENC_PCM_ENABLE                ENABLE
#define TCFG_ENC_SBC_ENABLE                ENABLE
#define TCFG_ENC_OPUS_ENABLE                DISABLE
#define TCFG_ENC_SPEEX_ENABLE               DISABLE


//*********************************************************************************//
//ali ai profile
//
#define DUEROS_DMA_EN              0  //not surport
#define TRANS_DATA_EN              0  //数传demo
#define SMART_BOX_EN               1

#if (DUEROS_DMA_EN || TRANS_DATA_EN || SMART_BOX_EN || RCSP_BTMATE_EN)
#undef  TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE	   1
#else
#endif

#if (SMART_BOX_EN)
#undef  TCFG_USER_EMITTER_ENABLE
#define TCFG_USER_EMITTER_ENABLE	0

#undef  RCSP_FILE_OPT
#define RCSP_FILE_OPT				0
#endif


//*********************************************************************************//
//                                 电源切换配置                                    //
//*********************************************************************************//

#define CONFIG_PHONE_CALL_USE_LDO15	    1


//*********************************************************************************//
//             启用三路ADC接口
//*********************************************************************************//
#if((TCFG_REVERB_ENABLE) && (LINEIN_INPUT_WAY == LINEIN_INPUT_WAY_ADC))
#define THREE_ADC_ENABLE
#endif


//*********************************************************************************//
//            解码叠加配置
//*********************************************************************************//
#define DEC_MIX_ENABLE						  0

//*********************************************************************************//
//                                人声消除使能
//*********************************************************************************//
#define AUDIO_VOCAL_REMOVE_EN       0


///*********************************************************************************//
//          等响度 开启后，需要固定模拟音量,调节软件数字音量
//          等响度使用eq实现，同个数据流中，若打开等响度，请开eq总使能，关闭其他eq,例如蓝牙模式eq
//*********************************************************************************//
#define AUDIO_EQUALLOUDNESS_CONFIG  0  //等响度

///*********************************************************************************//
//          环绕音效使能
//*********************************************************************************//
#define AUDIO_SURROUND_CONFIG     0//3d环绕

///*********************************************************************************//
//          虚拟低音使能，不支持四声道
//*********************************************************************************//
#define AUDIO_VBASS_CONFIG        0//虚拟低音,虚拟低音不支持四声道



//*********************************************************************************//
//                                 编译警告                                         //
//*********************************************************************************//
#if ((TRANS_DATA_EN ) && ((TCFG_PC_ENABLE || TCFG_UDISK_ENABLE) && ((TCFG_ONLINE_TX_PORT == IO_PORT_DP) && TCFG_ONLINE_ENABLE)))
#error "eq online adjust enable, plaease close usb marco !!!"
#endif// ((TRANS_DATA_EN || TCFG_ONLINE_ENABLE) && (TCFG_PC_ENABLE || TCFG_UDISK_ENABLE))

#if TCFG_UI_ENABLE
#if ((TCFG_SPI_LCD_ENABLE &&  TCFG_CODE_FLASH_ENABLE) && (TCFG_FLASH_DEV_SPI_HW_NUM == TCFG_TFT_LCD_DEV_SPI_HW_NUM))
#error "flash spi port == lcd spi port, please close one !!!"
#endif//((TCFG_SPI_LCD_ENABLE &&  TCFG_CODE_FLASH_ENABLE) && (TCFG_FLASH_DEV_SPI_HW_NUM == TCFG_TFT_LCD_DEV_SPI_HW_NUM))
#endif//TCFG_UI_ENABLE

#if(TRANS_DATA_EN && DUEROS_DMA_EN)
#error "they can not enable at the same time,just select one!!!"
#endif//(TRANS_DATA_EN && DUEROS_DMA_EN)

#if (TCFG_DEC2TWS_ENABLE && (TCFG_REVERB_ENABLE ||TCFG_APP_RECORD_EN || TCFG_APP_RTC_EN ||TCFG_DRC_ENABLE))
#error "对箱支持音源转发，请关闭混响录音等功能 !!!"
#endif// (TCFG_DEC2TWS_ENABLE && (TCFG_REVERB_ENABLE ||TCFG_APP_RECORD_EN || TCFG_APP_RTC_EN ||TCFG_DRC_ENABLE))

#if (TCFG_REVERB_ENABLE && (TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE))
#error "无损格式+混响暂时不支持同时打开 !!!"
#endif//(TCFG_REVERB_ENABLE && (TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE))

#if (TCFG_REVERB_DODGE_EN && (USER_DIGITAL_VOLUME_ADJUST_ENABLE == 0))
#error "使用闪避功能，打开自定义数字音量调节 !!!"
#endif

#if ((TCFG_APP_FM_EN) && (TCFG_MIC_EFFECT_ENABLE) && (RECORDER_MIX_EN))
#error "FM模式下开混响暂不支持混合录音"
#endif
///<<<<所有宏定义不要在编译警告后面定义！！！！！！！！！！！！！！！！！！！！！！！！！！
///<<<<所有宏定义不要在编译警告后面定义！！！！！！！！！！！！！！！！！！！！！！！！！！


//*********************************************************************************//
//                                 配置结束                                         //
//*********************************************************************************//


#endif //CONFIG_BOARD_AC695X_MULTIMEDIA_CHARGING_BIN
#endif //BOARD_AC695X_MULTIMEDIA_CHARGING_BIN_H
