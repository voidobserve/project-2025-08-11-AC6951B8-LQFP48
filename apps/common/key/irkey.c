#include "key_driver.h"
#include "irkey.h"
#include "gpio.h"
#include "asm/irflt.h"
#include "app_config.h"
#include "system/event.h"
#include "asm/uart_dev.h"
#include "includes.h"
#include "asm/cpu.h"
#include "asm/irq.h"
#include "asm/clock.h"
#include "system/init.h"
#include "debug.h"


// #if TCFG_IRKEY_ENABLE

#if 1


#define IR_CODE     0XFF  

u8 ir_get_key_value(void);

typedef struct _IR_KEY {
    u32 ir_addr;
	u16 ir_code;
	bool ir_flag;
	u32 ir_timeout;
}IR_KEY;

static IR_KEY ir_key;


//按键驱动扫描参数列表
struct key_driver_para irkey_scan_para = {
    .scan_time 	  	  = 2,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 1,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 0, //20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_IR,
    .get_value 		  = ir_get_key_value,
};



const u8 IRTabFF00[] = {
    NKEY_00, NKEY_01, NKEY_02, NKEY_03, NKEY_04, NKEY_05, NKEY_06, IR_06, IR_15, IR_08, NKEY_0A, NKEY_0B, IR_12, IR_11, NKEY_0E, NKEY_0F,
    NKEY_10, NKEY_11, NKEY_12, NKEY_13, NKEY_14, IR_07, IR_09, NKEY_17, IR_13, IR_10, NKEY_1A, NKEY_1B, IR_16, NKEY_1D, NKEY_1E, NKEY_1F,
    NKEY_20, NKEY_21, NKEY_22, NKEY_23, NKEY_24, NKEY_25, NKEY_26, NKEY_27, NKEY_28, NKEY_29, NKEY_2A, NKEY_2B, NKEY_2C, NKEY_2D, NKEY_2E, NKEY_2F,
    NKEY_30, NKEY_31, NKEY_32, NKEY_33, NKEY_34, NKEY_35, NKEY_36, NKEY_37, NKEY_38, NKEY_39, NKEY_3A, NKEY_3B, NKEY_3C, NKEY_3D, NKEY_3E, NKEY_3F,
    IR_04, NKEY_41, IR_18, IR_05, IR_03, IR_00, IR_01, IR_02, NKEY_48, NKEY_49, IR_20, NKEY_4B, NKEY_4C, NKEY_4D, NKEY_4E, NKEY_4F,
    NKEY_50, NKEY_51, IR_19, NKEY_53, NKEY_54, NKEY_55, NKEY_56, NKEY_57, NKEY_58, NKEY_59, IR_17, NKEY_5B, NKEY_5C, NKEY_5D, IR_14, NKEY_5F,
};

const u8 ir_tbl_FF00[][2] =
{

};

u8 ir_key_get(const u8 ir_table[][2], u8 ir_data)
{
    u8 keyval = NO_KEY;
	if(ir_key.ir_addr>>8 != IR_CODE)//遥控客户码
		return NO_KEY;
		keyval = ir_data;
	return keyval;
}

/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 ir_get_key_value(void)
{
    // u8 tkey = 0xff;
    // tkey = get_irflt_value();
    // if (tkey == 0xff) {
    //     return tkey;
    // }
    // tkey = IRTabFF00[tkey];
    // return tkey;


  u8 tkey = NO_KEY;

//这个超时，保持在70ms
	if(ir_key.ir_timeout < 50)  // 20 * 4ms  == 100
	{
		ir_key.ir_timeout++;
	}
	else    
	{
		ir_key.ir_addr= 0;
		ir_key.ir_code = 0;
		ir_key.ir_timeout = 0xff;//time out
		ir_key.ir_flag = 0;
	}

	// printf("ir_key.ir_flag = %d\n",ir_key.ir_flag);
	if(ir_key.ir_flag)
	{
		tkey = ir_key_get(ir_tbl_FF00, ir_key.ir_code);
		// printf("tkey = %d", tkey);
	}

    return tkey;




}

void user_ir_init(u8 port)
{
	gpio_set_die(port, 1);
	gpio_set_direction(port, 1);
	gpio_set_pull_up(port, 1);
}

/*----------------------------------------------------------------------------*/
/**@brief   ir按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ir_key_init(void)
*/
/*----------------------------------------------------------------------------*/
int irkey_init(const struct irkey_platform_data *irkey_data)
{
    printf("irkey_init ");

    // ir_input_io_sel(irkey_data->port);

    // ir_output_timer_sel();

    // irflt_config();

    // ir_timeout_set();

    user_ir_init(irkey_data->port);



    return 0;
}





u8 ir_get_key_null(void)
{
	return NO_KEY;
}



AT_VOLATILE_RAM_CODE
void ir_detect_isr(void)     //该API，不动，如识别解释不到键值，优先考虑定时器的时间
{

	static bool last_status = 1,ir_data_bit;
	static u8 soft_ir_cnt = 0, time_cnt = 250;
	static u32 ir_data_temp;
	bool new_status;

	//if(gpio_read(TCFG_IRKEY_PORT))
	if(JL_PORTA->IN & BIT(8)) //
	{
		new_status = 1;
		//JL_PORTB->OUT |= BIT(3); //输出1
	}
	else
	{
		new_status = 0;
		//JL_PORTB->OUT &= ~BIT(3); //输出0
	}


	if(new_status!=last_status)
	{
		last_status = new_status;
		if(new_status==0)
		{
			//if((time_cnt>100)&&(time_cnt<250))
			//printf("%d\n",time_cnt);
			if(((time_cnt>100)&&(time_cnt<150))//13.5ms 红外头 //11.25ms 连发码
			// ||((time_cnt>112)&&(time_cnt<123))
			)
			{
				soft_ir_cnt = 0;
				time_cnt = 1;
				ir_data_temp = 0;
				// printf("-");
				//JL_PORTB->OUT &= ~BIT(3); //输出0
				ir_key.ir_timeout = 0;
				return;
			}
			else if((time_cnt>8)&&(time_cnt<15))// 1.125ms	 bit0
			{
				ir_data_bit = 0;
				//JL_PORTB->OUT &= ~BIT(3); //输出0
				//printf("0");
			}
			else if((time_cnt>18)&&(time_cnt<30))// 2.25ms	 bit1
			{
				ir_data_bit = 1;
				//JL_PORTB->OUT &= ~BIT(3); //输出0
				//printf("1");
			}
			else
			{
				time_cnt =1;
				return;
			}
			time_cnt = 1;

			ir_data_temp >>= 1;
			soft_ir_cnt++;

			if(ir_data_bit)
			{
				ir_data_temp |= 0x80000000;
				//printf("1");
			}
			else
			{
				//printf("0");
			}

			//printf("soft_ir_cnt = %d\n",soft_ir_cnt);
			if (soft_ir_cnt == 32)
			{
				ir_key.ir_addr= ir_data_temp&0xffff;
				ir_key.ir_code= (ir_data_temp>>16)&0xff;
				ir_key.ir_flag = 1;
				printf("code = %x\n",ir_key.ir_code);
				// printf("%X ",ir_key.ir_addr);
			}
		}
	}
	else
	{
		if(time_cnt<250)
		{
			time_cnt++;
		}
	}
}


#endif









