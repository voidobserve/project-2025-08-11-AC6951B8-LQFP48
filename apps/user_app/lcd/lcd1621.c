#include "includes.h"
#include "app_config.h"
#include "app_task.h"

#include "ui/ui_api.h"

#if 1  // USER_UI_1621LCD_ENABLE

#include "lcd1621.h"
#include "adkey.h"

#include "../../../apps/user_app/user_config.h"
#include "user_sys_time.h"

//////////////////显示命令字典/////////////////////
#define CMD_BYTE   8
#define MODE_BYTE  3
#define DATA_BYTE  4
#define ADDR_BYTE  6
#define MODE_CMD   0x80
#define MODE_WRITE 0xa0

#define SYS_EN          0x01
#define SYS_DIS         0x00
#define SYS_DIS         0x00
#define LCD_ON_1621     0x03
#define RC_256K         0x18
#define IRQ_DIS         0x80
#define LCD_OFF_1621    0x02
#define BIAS_1_3        0x29
#define BIAS_1_2        0x28

////////////////////////片选线///////////////////////////////////
#define SETB_OUT()              JL_PORTB->DIR &= ~BIT(9)

#define SETB_HIGH               JL_PORTB->OUT |=  BIT(9)
#define SETB_LOW                JL_PORTB->OUT &= ~BIT(9)

////////////////////////////////////////////////////////////////

////////////////////////数据线///////////////////////////////////
#define Sdata_Ouput_1621()      JL_PORTB->DIR &= ~BIT(11)

#define Sdata_h_1621()          JL_PORTB->OUT |=  BIT(11)
#define Sdata_l_1621()          JL_PORTB->OUT &= ~BIT(11)
////////////////////////////////////////////////////////////////

////////////////////////时钟线///////////////////////////////////
#define Sclk_Ouput_1621()       JL_PORTB->DIR &= ~BIT(10)

#define Sclk_h_1621()           JL_PORTB->OUT |=  BIT(10)
#define Sclk_l_1621()           JL_PORTB->OUT &= ~BIT(10)
////////////////////////////////////////////////////////////////

//#define LCD1621_IO_INIT()			GPIOADE |= (BIT(2)|BIT(3)|BIT(4))

// u8 lcd1621_sendbuf[16] = { 0 };   ///2020-12-10 lcd1621传输中转


// 定义SEG的脚位，从左到右数字顺序分别是1、2、3...6
// 最多7个byte，不足7个byte的以0xFF结束
u16 const num1[10][7] =
{
	{SEG_1A, SEG_1B, SEG_1C, SEG_1D, SEG_1E, SEG_1F, 0xFFFF}, //0
	{SEG_1B, SEG_1C, 0xFFFF}, //1
	{SEG_1A, SEG_1B,SEG_1E,SEG_1D,SEG_1G, 0xFFFF },
	{SEG_1A, SEG_1B,SEG_1C,SEG_1D,SEG_1G, 0xFFFF },
	{SEG_1B, SEG_1C,SEG_1F,SEG_1G, 0xFFFF },
	{SEG_1A, SEG_1F,SEG_1C,SEG_1D,SEG_1G , 0xFFFF},
	{SEG_1A, SEG_1F,SEG_1E,SEG_1D,SEG_1C,SEG_1G, 0xFFFF },
	{SEG_1A, SEG_1B,SEG_1C, 0xFFFF},
	{SEG_1A, SEG_1B,SEG_1C,SEG_1D,SEG_1E,SEG_1F,SEG_1G },
	{SEG_1A, SEG_1B,SEG_1C,SEG_1D,SEG_1F,SEG_1G , 0xFFFF},//9
};

u16 const num2[10][7] =
{
	{SEG_2A, SEG_2B, SEG_2C, SEG_2D, SEG_2E, SEG_2F,0xFFFF}, //0
	{SEG_2B, SEG_2C, 0xFFFF}, //2
	{SEG_2A, SEG_2B,SEG_2E,SEG_2D,SEG_2G , 0xFFFF},
	{SEG_2A, SEG_2B,SEG_2C,SEG_2D,SEG_2G , 0xFFFF},
	{SEG_2B, SEG_2C,SEG_2F,SEG_2G , 0xFFFF},
	{SEG_2A, SEG_2F,SEG_2C,SEG_2D,SEG_2G , 0xFFFF},
	{SEG_2A, SEG_2F,SEG_2E,SEG_2D,SEG_2C,SEG_2G, 0xFFFF },
	{SEG_2A, SEG_2B,SEG_2C, 0xFFFF},
	{SEG_2A, SEG_2B,SEG_2C,SEG_2D,SEG_2E,SEG_2F,SEG_2G },//8
	{SEG_2A, SEG_2B,SEG_2C,SEG_2D,SEG_2F,SEG_2G, 0xFFFF },//9
};

u16 const num3[10][7] =
{
	{SEG_3A, SEG_3B, SEG_3C, SEG_3D, SEG_3E, SEG_3F,0xFFFF}, //0
	{SEG_3B, SEG_3C, 0xFFFF}, //3
	{SEG_3A, SEG_3B,SEG_3E,SEG_3D,SEG_3G, 0xFFFF },
	{SEG_3A, SEG_3B,SEG_3C,SEG_3D,SEG_3G , 0xFFFF},
	{SEG_3B, SEG_3C,SEG_3F,SEG_3G , 0xFFFF},
	{SEG_3A, SEG_3F,SEG_3C,SEG_3D,SEG_3G , 0xFFFF},
	{SEG_3A, SEG_3F,SEG_3E,SEG_3D,SEG_3C,SEG_3G, 0xFFFF },
	{SEG_3A, SEG_3B,SEG_3C, 0xFFFF},
	{SEG_3A, SEG_3B,SEG_3C,SEG_3D,SEG_3E,SEG_3F,SEG_3G },
	{SEG_3A, SEG_3B,SEG_3C,SEG_3D,SEG_3F,SEG_3G, 0xFFFF },//9
};

u16 const num4[10][7] =
{
	{SEG_4A, SEG_4B, SEG_4C, SEG_4D, SEG_4E, SEG_4F,0xFFFF}, //0
	{SEG_4B, SEG_4C, 0xFFFF},                                //1
	{SEG_4A, SEG_4B,SEG_4E,SEG_4D,SEG_4G, 0xFFFF },          //2
	{SEG_4A, SEG_4B,SEG_4C,SEG_4D,SEG_4G , 0xFFFF},
	{SEG_4B, SEG_4C,SEG_4F,SEG_4G, 0xFFFF },
	{SEG_4A, SEG_4F,SEG_4C,SEG_4D,SEG_4G , 0xFFFF},
	{SEG_4A, SEG_4F,SEG_4E,SEG_4D,SEG_4C,SEG_4G , 0xFFFF},   //6
	{SEG_4A, SEG_4B,SEG_4C, 0xFFFF},                         //7
	{SEG_4A, SEG_4B,SEG_4C,SEG_4D,SEG_4E,SEG_4F,SEG_4G }, //8
	{SEG_4A, SEG_4B,SEG_4C,SEG_4D,SEG_4F,SEG_4G , 0xFFFF},  //9
};

u16 const num5[10][7] =
{
	{SEG_5A, SEG_5B, SEG_5C, SEG_5D, SEG_5E, SEG_5F,0xFFFF}, //0
	{SEG_5B, SEG_5C, 0xFFFF}, //5
	{SEG_5A, SEG_5B,SEG_5E,SEG_5D,SEG_5G, 0xFFFF },
	{SEG_5A, SEG_5B,SEG_5C,SEG_5D,SEG_5G, 0xFFFF },
	{SEG_5B, SEG_5C,SEG_5F,SEG_5G, 0xFFFF },
	{SEG_5A, SEG_5F,SEG_5C,SEG_5D,SEG_5G, 0xFFFF },
	{SEG_5A, SEG_5F,SEG_5E,SEG_5D,SEG_5C,SEG_5G, 0xFFFF },
	{SEG_5A, SEG_5B,SEG_5C, 0xFFFF},
	{SEG_5A, SEG_5B,SEG_5C,SEG_5D,SEG_5E,SEG_5F,SEG_5G },
	{SEG_5A, SEG_5B,SEG_5C,SEG_5D,SEG_5F,SEG_5G , 0xFFFF},//9
};

u16 const num6[10][7] =
{
	{SEG_6A, SEG_6B, SEG_6C, SEG_6D, SEG_6E, SEG_6F,0xFFFF}, //0
	{SEG_6B, SEG_6C, 0xFFFF}, //6
	{SEG_6A, SEG_6B, SEG_6E, SEG_6D, SEG_6G, 0xFFFF },
	{SEG_6A, SEG_6B, SEG_6C, SEG_6D, SEG_6G, 0xFFFF},
	{SEG_6B, SEG_6C, SEG_6F, SEG_6G, 0xFFFF},
	{SEG_6A, SEG_6F, SEG_6C, SEG_6D, SEG_6G, 0xFFFF},
	{SEG_6A, SEG_6F, SEG_6E, SEG_6D, SEG_6C,SEG_6G , 0xFFFF},
	{SEG_6A, SEG_6B, SEG_6C, 0xFFFF},
	{SEG_6A, SEG_6B, SEG_6C, SEG_6D, SEG_6E,SEG_6F,SEG_6G },
	{SEG_6A, SEG_6B, SEG_6C, SEG_6D, SEG_6F,SEG_6G , 0xFFFF},//9
};

u16 const num7[10][7] =
{
	{SEG_7A, SEG_7B, SEG_7C, SEG_7D, SEG_7E, SEG_7F,0xFFFF}, //0
	{SEG_7B, SEG_7C, 0xFFFF}, //6
	{SEG_7A, SEG_7B, SEG_7E, SEG_7D, SEG_7G, 0xFFFF },
	{SEG_7A, SEG_7B, SEG_7C, SEG_7D, SEG_7G, 0xFFFF},
	{SEG_7B, SEG_7C, SEG_7F, SEG_7G, 0xFFFF},
	{SEG_7A, SEG_7F, SEG_7C, SEG_7D, SEG_7G, 0xFFFF},
	{SEG_7A, SEG_7F, SEG_7E, SEG_7D, SEG_7C,SEG_7G , 0xFFFF},
	{SEG_7A, SEG_7B, SEG_7C, 0xFFFF},
	{SEG_7A, SEG_7B, SEG_7C, SEG_7D, SEG_7E,SEG_7F,SEG_7G },
	{SEG_7A, SEG_7B, SEG_7C, SEG_7D, SEG_7F,SEG_7G , 0xFFFF},//9
};

/*
	存放第一位数码管显示对应符号的表格
	顺序：第一位数码管 第二位数码管 第三位数码管
		第四位 第五位 第六位 第七位

	设置继电器的定时计划时，
	第一位数码管需要显示的字符：
*/
const u16 seg1_show_relay_schedule_table[][7] =
{
	{SEG_1A, SEG_1B, SEG_1E, SEG_1F, SEG_1G }, // 数码管显示 大写P/小写p
	{SEG_1A, SEG_1E, SEG_1F, SEG_1G }, // 数码管显示 大写F
};


volatile unsigned char  voltage_array[3] = { 0 }; // 存放要显示的电压
volatile unsigned char dis_data[32]; // 显存


void delay_lcd(u8 n)
{
	while (n--)
	{
		asm("nop");
		asm("nop");
	}
}

void lcd1621_write_byte(u8 datas, u8 len)
{
	u8 cnt;
	Sdata_Ouput_1621();
	Sclk_Ouput_1621();
	delay_lcd(10);
	cnt = len;
	delay_lcd(10);
	while (cnt > 0)
	{
		Sclk_l_1621();
		delay_lcd(10);
		if ((datas & 0x80) == 0x80)
			Sdata_h_1621();
		else
			Sdata_l_1621();
		delay_lcd(10);
		Sclk_h_1621();
		datas <<= 1;
		cnt--;
		delay_lcd(10);
	}

	if (len == CMD_BYTE)
	{
		Sclk_l_1621();
		Sdata_l_1621();
		delay_lcd(10);
		Sclk_h_1621();
		delay_lcd(10);
	}
}

void lcd1621_write_cmd(u8 cmd)
{
	delay_lcd(5);
	SETB_LOW;
	delay_lcd(10);
	lcd1621_write_byte(MODE_CMD, MODE_BYTE);          //写入100命令模式
	lcd1621_write_byte(cmd, CMD_BYTE);
	delay_lcd(10);
	SETB_HIGH;
	delay_lcd(5);
}



//写数据
void lcd1621_write_data(u8* buff, u8 len)
{
	u8 addr = 0;
	u8 i;
	delay_lcd(5);
	SETB_LOW;
	delay_lcd(10);
	lcd1621_write_byte(MODE_WRITE, MODE_BYTE);  //写模式101
	lcd1621_write_byte(addr, ADDR_BYTE);        //数据写入地址
	for (i = 0;i < 32; i++) //要写入的数据
	{
		// lcd1621_write_byte(lcd1621_sendbuf[i]>>8, DATA_BYTE);
		// lcd1621_write_byte(lcd1621_sendbuf[i]>>4, DATA_BYTE);
		// lcd1621_write_byte(lcd1621_sendbuf[i], DATA_BYTE);
		// lcd1621_write_byte(lcd1621_sendbuf[i]<<4, DATA_BYTE);

		// lcd1621_write_byte(lcd1621_sendbuf[i], DATA_BYTE);
		// lcd1621_write_byte(lcd1621_sendbuf[i]<<4, DATA_BYTE);
		// if (buff)
		// {
		lcd1621_write_byte(buff[i], DATA_BYTE);
		// lcd1621_write_byte(buff[i] << 4, DATA_BYTE);
	// }
	// else
	// {
	//     lcd1621_write_byte(0, DATA_BYTE);
	//     lcd1621_write_byte(0, DATA_BYTE);
	// }
	}
	delay_lcd(10);
	SETB_HIGH;
	delay_lcd(5);
}

void lcd1621_reset(void)
{
	SETB_OUT();
	Sclk_Ouput_1621();
	Sdata_Ouput_1621();
	delay_lcd(5);
	lcd1621_write_cmd(SYS_EN);
	lcd1621_write_cmd(RC_256K);         //选取片内RC晶振
	lcd1621_write_cmd(BIAS_1_3);       //1/3偏压,4个COM端
	// lcd1621_write_cmd(0x21);       // 1/3偏压,2个COM端（屏幕会有一半的图标没点亮）
	// lcd1621_write_cmd(BIAS_1_2);       //1/2偏压,4个COM端 (会在侧面看到其他没有点亮的图标)
	lcd1621_write_cmd(IRQ_DIS);        //时基电路失效（看门狗失效）
	lcd1621_write_cmd(LCD_ON_1621);           //点着显示屏
}

void lcd1621_value_set(u8* buff, u8 len)
{
	lcd1621_reset();
	lcd1621_write_data(buff, len);

}

// /**
//  * @brief 立即刷新lcd显示，把缓冲区的数据都发送给lcd驱动ic
//  *		注意，不能在外部调用强制刷新，会导致数据冲突，lcd任务是定时器调用
//  *
//  */
// void lcd_1621_refresh(void)
// {
// 	if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
// 	{
// 		lcd_setting_sys_time_animation();
// 	}

// 	lcd1621_write_data(dis_data, 16); // 将lcd显示缓冲区的数据发送给屏幕驱动ic
// }


// u8 display_data[16] = { 0 };

void lcd1621_init(void)
{
	// LCD1621_IO_INIT();
	lcd1621_value_set(NULL, 16);

	//lcd屏的背光灯
	gpio_set_pull_down(lcd_light, 0);
	gpio_set_pull_up(lcd_light, 0);
	gpio_direction_output(lcd_light, 0);
	gpio_direction_output(lcd_light, 0); //背光灯默认关


	// gpio_direction_output(lcd_light, 1); // 打开背光 -- 测试时使用
}



/*
	存放第一位数码管显示对应符号的表示

	设置系统时间时，
	第一位数码管需要显示的字符：
*/
// const u16 seg1_show_sys_time_table[][7] =
// {
// 	{SEG_1B, SEG_1C, SEG_1D, SEG_1F, SEG_1G, 0xFFFF}, // 数码管显示 小写y ，表示当前正在设置 年份
// 	{SEG_1A, SEG_1B, SEG_1C, SEG_1D, SEG_1E, SEG_1F, 0xFFFF}, // 数码管显示 O ，表示当前正在设置 月份
// 	{SEG_1B, SEG_1C, SEG_1D, SEG_1E, SEG_1G, 0xFFFF }, // 数码管显示 小写d ，表示当前正在设置 日期	
// 	{SEG_1B, SEG_1C, SEG_1E, SEG_1F, SEG_1G, 0xFFFF},// 数码管显示 大写H ，表示当前正在设置 小时
// 	{},// 数码管显示 小写t ，表示当前正在设置 分钟
// }


// 构造显存数据,d符合 SEG_SET 的规则
void make_dis(u16 d)
{
	// printf("%04x",d);
		//     第几个seg           第几个com
	dis_data[d & SEG_MASK] |= ((d & COM_MASK) >> 4);
}

void clean_dis(u16 d)
{
	// printf("%04x",d);
	dis_data[d & SEG_MASK] &= ((d & COM_MASK) >> 4);
}

void lcd1621_dispbuff_clr(void)
{
	u8 i;
	for (i = 0;i < 32;i++)
	{
		dis_data[i] = 0;
	}
}

// num 第几个数字
// dec显示的数字
void make_num(unsigned char num, unsigned char dec)
{
	unsigned char i;
	for (i = 0; i < 7; i++) // 七段数码管
	{
		if (num == 1)
		{
			if (num1[dec][i] == 0xFFFF) break;
			make_dis(num1[dec][i]);
		}

		if (num == 2)
		{
			if (num2[dec][i] == 0xFFFF) break;
			make_dis(num2[dec][i]);
		}

		if (num == 3)
		{
			if (num3[dec][i] == 0xFFFF) break;
			make_dis(num3[dec][i]);
		}

		if (num == 4)
		{
			if (num4[dec][i] == 0xFFFF) break;
			make_dis(num4[dec][i]);
		}

		if (num == 5)
		{
			if (num5[dec][i] == 0xFFFF) break;
			make_dis(num5[dec][i]);
		}

		if (num == 6)
		{
			if (num6[dec][i] == 0xFFFF) break;
			make_dis(num6[dec][i]);
		}
		if (num == 7)
		{
			if (num7[dec][i] == 0xFFFF) break;
			make_dis(num7[dec][i]);
		}
	}
}

/**
 * @brief lcd 在 第一位 数码管显示字母
 *
 * @param alphabet_index 字母在表格 seg1_show_relay_schedule_table 中的索引
 */
void lcd_show_alphabet_in_seg_1(u8 alphabet_index)
{
	for (u8 i = 0; i < 7; i++)
	{
		make_dis(seg1_show_relay_schedule_table[alphabet_index][i]);
	}
}
 

//开机时，显示轮廓
void lcd_open_frame(void)
{

	// unsigned char i;
	// static u16 cnt_ = 20000;

	lcd1621_write_cmd(LCD_ON_1621);
	// os_time_dly(1000);
	make_dis(SEG_S5); // "V"
	// make_dis(SEG_S6); // "W"
	// make_dis(SEG_T1); // 继电器通道边框
	// make_dis(SEG_T);   // 音符
	// make_dis(SEG_S2);   // 交流

	// make_dis(SEG_X2);  // 锁符号的下半部分
	// make_dis(SEG_X3); // 锁符号的上半部分

	// 数码管：
	make_num(1, 8); // 第一位数码管，显示8
	make_num(2, 8); // 第二位数码管，显示8
	make_num(3, 8);
	// make_num(4, 8); 
	// make_num(5, 8); 
	// make_num(6, 8); 
	// make_num(7, 8); 

	gpio_direction_output(lcd_light, 1); //背光灯
}

// 点亮继电器通道边框，点亮背光
void liangbiankuang(void)
{
	gpio_direction_output(lcd_light, 1); //背光灯
	lcd1621_write_cmd(LCD_ON_1621);
	make_dis(SEG_T1); // 继电器通道边框
}

//检查屏幕是否有坏
void check_lcd_display(void)
{
	u8 i = 0;
	for (i = 0; i < 32; i++)
	{
		dis_data[i] = 0xff;
	}
}

void lcd1621_off(void)
{
	lcd1621_write_cmd(LCD_OFF_1621);
}




/**
 * @brief LCD显示单个继电器图标
 *
 * @param relay_index 继电器的索引
 */
void lcd_show_relay_icon(u8 relay_index)
{
	switch (relay_index)
	{
	case 0:make_dis(SEG_1);break;
	case 1:make_dis(SEG_2);break;
	case 2:make_dis(SEG_3);break;
	case 3:make_dis(SEG_4);break;
	case 4:make_dis(SEG_5);break;
	case 5:make_dis(SEG_6);break;
	case 6:make_dis(SEG_7);break;
	case 7:make_dis(SEG_8);break;
	}
}

/**
 * @brief lcd 显示 对应的继电器图标
 *
 * @param relay_index 继电器的索引值
 * @return * void
 */
void lcd_relay_icon_show(relay_index_t relay_index)
{
	switch (relay_index)
	{
	case RELAY_INDEX_0: make_dis(SEG_1); break;
	case RELAY_INDEX_1: make_dis(SEG_2); break;
	case RELAY_INDEX_2: make_dis(SEG_3); break;
	case RELAY_INDEX_3: make_dis(SEG_4); break;
	case RELAY_INDEX_4: make_dis(SEG_5); break;
	case RELAY_INDEX_5: make_dis(SEG_6); break;
	case RELAY_INDEX_6: make_dis(SEG_7); break;
	case RELAY_INDEX_7: make_dis(SEG_8); break;
	}
}

/**
 * @brief lcd 不显示 对应的继电器图标
 *
 * @param relay_index 继电器的索引值
 * @return * void
 */
void lcd_relay_icon_unshow(relay_index_t relay_index)
{
	switch (relay_index)
	{
	case RELAY_INDEX_0: clean_dis(clrbit(SEG_1)); break;
	case RELAY_INDEX_1: clean_dis(clrbit(SEG_2)); break;
	case RELAY_INDEX_2: clean_dis(clrbit(SEG_3)); break;
	case RELAY_INDEX_3: clean_dis(clrbit(SEG_4)); break;
	case RELAY_INDEX_4: clean_dis(clrbit(SEG_5)); break;
	case RELAY_INDEX_5: clean_dis(clrbit(SEG_6)); break;
	case RELAY_INDEX_6: clean_dis(clrbit(SEG_7)); break;
	case RELAY_INDEX_7: clean_dis(clrbit(SEG_8)); break;
	}
}



//灭指定段
u16 clrbit(u16 x)
{
	u8 x_L, x_H;
	u16 result;

	// x &= ~(1<<y); //指定Y位为0
	x_L = x;
	x_H = x >> 8;
	x_H = ~x_H;
	result = (x_H << 8) | x_L;

	return result;
}



//关闭lcd的继电器通道显示
// void adkey_ctrl_lcd_relays_close(u8 relay_number)
/**
 * @brief LCD清除单个继电器图标(不显示对应的继电器图标)
 *
 * @param relay_index 单个继电器的索引
 */
void lcd_clear_relay_icon(u8 relay_index)
{
	switch (relay_index)
	{
	case 0:clean_dis(clrbit(SEG_1));break;
	case 1:clean_dis(clrbit(SEG_2));break;
	case 2:clean_dis(clrbit(SEG_3));break;
	case 3:clean_dis(clrbit(SEG_4));break;
	case 4:clean_dis(clrbit(SEG_5));break;
	case 5:clean_dis(clrbit(SEG_6));break;
	case 6:clean_dis(clrbit(SEG_7));break;
	case 7:clean_dis(clrbit(SEG_8));break;

	}
}





//熄灭功率显示的数字
//更新数据时，需要灭
void clean_num(unsigned char num)
{
	unsigned char i;

	for (i = 0; i < 7; i++)// 七段数码管
	{
		if (num == 1)
		{
			clean_dis(clrbit(num1[8][i]));
		}
		if (num == 2)
		{
			clean_dis(clrbit(num2[8][i]));
		}
		if (num == 3)
		{
			clean_dis(clrbit(num3[8][i]));
		}
		if (num == 4)
		{
			clean_dis(clrbit(num4[8][i]));
		}
		if (num == 5)
		{
			clean_dis(clrbit(num5[8][i]));
		}
		if (num == 6)
		{
			clean_dis(clrbit(num6[8][i]));
		}
		if (num == 7)
		{
			clean_dis(clrbit(num7[8][i]));
		}
	}
}



void lcd_update_ac_voltage(void)
{
	// 交流电电压：
	make_num(1, voltage_array[0]);
	make_num(2, voltage_array[1]);
	make_num(3, voltage_array[2]);
}

void lcd_clear_ac_voltage(void)
{
	// 清除第一位~第三位数码管显示的内容
	clean_num(1);
	clean_num(2);
	clean_num(3);
}


// lcd显示年份
void lcd_update_year(const u16 year)
{
	u8 bit_0 = 0; // lcd显示时间部分的第0位数据，范围：0~9
	u8 bit_1 = 0;
	u8 bit_2 = 0;
	u8 bit_3 = 0;

	// u16 tmp = year;
	bit_0 = (u8)((u16)year / 1000);
	bit_1 = (u8)((u16)year / 100 % 10);
	bit_2 = (u8)((u16)year / 10 % 10);
	bit_3 = (u8)((u16)year % 10);

	make_num(4, bit_0);
	make_num(5, bit_1);
	make_num(6, bit_2);
	make_num(7, bit_3);
}

// lcd不显示年份
void lcd_clear_year(void)
{
	clean_num(4);
	clean_num(5);
	clean_num(6);
	clean_num(7);
}

void lcd_update_month(const u8 month)
{
	u8 bit_0 = 0; // lcd显示时间部分的第0位数据，范围：0~9
	u8 bit_1 = 0;

	bit_0 = month / 10;
	bit_1 = month % 10;

	make_num(4, bit_0);
	make_num(5, bit_1);
}

void lcd_clear_month(void)
{
	clean_num(4);
	clean_num(5);
}

void lcd_update_day(const u8 day)
{
	u8 bit_0 = 0; // lcd显示时间部分的第0位数据，范围：0~9
	u8 bit_1 = 0;

	bit_0 = day / 10;
	bit_1 = day % 10;

	make_num(6, bit_0);
	make_num(7, bit_1);
}

void lcd_clear_day(void)
{
	clean_num(6);
	clean_num(7);
}

void lcd_update_hour(const u8 hour)
{
	u8 bit_0 = 0; // lcd显示时间部分的第0位数据，范围：0~9
	u8 bit_1 = 0;

	bit_0 = hour / 10;
	bit_1 = hour % 10;

	make_num(4, bit_0);
	make_num(5, bit_1);
}

void lcd_clear_hour(void)
{
	clean_num(4);
	clean_num(5);
}

void lcd_update_min(const u8 min)
{
	u8 bit_0 = 0; // lcd显示时间部分的第0位数据，范围：0~9
	u8 bit_1 = 0;

	bit_0 = min / 10;
	bit_1 = min % 10;

	make_num(6, bit_0);
	make_num(7, bit_1);
}

void lcd_clear_min(void)
{
	clean_num(6);
	clean_num(7);
}



















#endif
