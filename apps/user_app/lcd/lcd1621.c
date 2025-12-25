#include "includes.h"
#include "app_config.h"
#include "app_task.h"

#include "ui/ui_api.h"

#if 1  // USER_UI_1621LCD_ENABLE

#include "lcd1621.h"

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



// volatile u8 cur_setting_time_unit = 

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

/**
 * @brief 立即刷新lcd显示，把缓冲区的数据都发送给lcd驱动ic
 *		注意，不能在外部调用强制刷新，会导致数据冲突，lcd任务是定时器调用
 *
 */
void lcd_1621_refresh(void)
{
	if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
	{
		lcd_setting_sys_time_animation();
	}

	lcd1621_write_data(dis_data, 16); // 将lcd显示缓冲区的数据发送给屏幕驱动ic
}


// u8 display_data[16] = { 0 };
#include "adkey.h"
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

volatile unsigned char dis_data[32];
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



// 构造显存数据,d符合SEG_SET的规则
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
	for (i = 0; i < 7; i++)
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



u16 const alphabet_1[10][7] =
{
	{SEG_1A, SEG_1B,SEG_1E,SEG_1F,SEG_1G, 0xFFFF }, //P
	{SEG_1A, SEG_1D,SEG_1E,SEG_1F, 0xFFFF },        //C
	{SEG_1A, SEG_1D,SEG_1E,SEG_1F,SEG_1G, 0xFFFF }, //E
	{SEG_1A, SEG_1G,SEG_1E,SEG_1F, 0xFFFF },        //F 
};

void make_alphabet(u8 p)
{
	unsigned char i;
	for (i = 0; i < 7; i++)
	{
		make_dis(alphabet_1[p][i]);
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

	// 数字
	make_num(1, 8); // 第一位数字，显示8
	make_num(2, 8); // 第二位数字，显示8
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

	for (i = 0; i < 7; i++)
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




// u8 blink_f = 0; // 控制闪烁时间间隔的标志位
// u16 blink_cnt = 0;
// u8 lcd_now_state = set_sys_time;  // 上电默认设置系统时间
// u8 lcd_now_state = show_power;  // 上电默认显示交流电电压
extern u8 time_unit;
extern u8 sys_time_unit;
extern u8 split_open_time[8][4];
extern u8 split_close_time[8][4];
extern u8 chose_relays_num;

unsigned char voltage_array[3] = { 0 }; // 存放要显示的电压
unsigned char  power_array[4] = { 0 }; // 存放要显示的功率

u16 update_cnt = 0;

// extern struct sys_time sys_current_time; 
// extern struct sys_time sys_setting_time; 

extern u8 temp_year[4];
extern u8 temp_month[2];
extern u8 temp_day[2];
extern u8 temp_hour[2];
extern u8 temp_min[2];
extern u8 temp_sec[2];

extern u8 set_countdown_open_year[8][4];
extern u8 set_countdown_open_month[8][2];
extern u8 set_countdown_open_day[8][2];
extern u8 set_countdown_open_hour[8][2];
extern u8 set_countdown_open_min[8][2];
extern u8 set_countdown_open_sec[8][2];

extern u8 set_countdown_close_year[8][4];
extern u8 set_countdown_close_month[8][2];
extern u8 set_countdown_close_day[8][2];
extern u8 set_countdown_close_hour[8][2];
extern u8 set_countdown_close_min[8][2];
extern u8 set_countdown_close_sec[8][2];


// u8 BT_CONNECT_STATE = 0;

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



// 定义在设置系统时间期间，lcd动画使用到的各个变量
typedef struct
{
	user_sys_time_t cur_setting_sys_time; // 当前正在设置的系统时间
	time_unit_t cur_setting_time_unit; // 当前正在设置的时间单位
	u16 blink_cnt; // 闪烁时间的计数值，控制每个一段时间闪烁一次要显示的、正在设置的系统时间
	u8 blink_dir; // 闪烁方向，0：常亮，1：熄灭（不显示） 
} lcd_setting_sys_time_t;

volatile lcd_setting_sys_time_t lcd_setting_sys_time = { 0 };

static volatile u16 lcd_setting_sys_time_blink_cnt = 0;
static volatile u8 lcd_setting_sys_time_blink_dir = 0;
static volatile u8 lcd_setting_sys_time_unit = TIME_UNIT_YEAR; // 存放正在设置的时间单位  

void lcd_setting_sys_time_unit_change(time_unit_t time_unit)
{
	lcd_setting_sys_time_unit = time_unit;
}

void lcd_setting_sys_time_unit_get(time_unit_t* time_unit)
{
	*time_unit = lcd_setting_sys_time_unit;
}

// lcd显示系统时间动画，会不让时间闪烁，固定显示一段时间
void lcd_setting_sys_time_animation_fix(void)
{
	// lcd_setting_sys_time.blink_cnt = 0;
	// lcd_setting_sys_time.blink_dir = 0;

	lcd_setting_sys_time_blink_cnt = 0;
	lcd_setting_sys_time_blink_dir = 0;

	// if (TIME_UNIT_YEAR == lcd_setting_sys_time_unit)
	// {
	// 	lcd_update_year(cur_setting_sys_time.year);
	// }
	// else if (TIME_UNIT_MONTH == lcd_setting_sys_time_unit ||
	// 	TIME_UNIT_DAY == lcd_setting_sys_time_unit)
	// {
	// 	lcd_update_month(cur_setting_sys_time.month);
	// 	lcd_update_day(cur_setting_sys_time.day);
	// }
	// else if (TIME_UNIT_HOUR == lcd_setting_sys_time_unit ||
	// 	TIME_UNIT_MIN == lcd_setting_sys_time_unit)
	// {
	// 	lcd_update_hour(cur_setting_sys_time.hour);
	// 	lcd_update_min(cur_setting_sys_time.min);
	// }
}

void lcd_setting_sys_time_animation(void)
{
	// 如果正在设置系统时间
	// 让正在设置的时间闪烁，1 ： 年 ，2：月-日，3：时：分

	lcd_setting_sys_time_blink_cnt++;
	// if (lcd_setting_sys_time_blink_cnt <= 50) // 调用时间10ms，那么这里的时间单位就是10ms
	// {
	// 	return;
	// }

	// lcd_setting_sys_time_blink_cnt = 0;
	// lcd_setting_sys_time_blink_dir = !lcd_setting_sys_time_blink_dir;

	if (lcd_setting_sys_time_blink_cnt >= 50)// 调用时间10ms，那么这里的时间单位就是10ms
	{
		lcd_setting_sys_time_blink_cnt = 0;
		lcd_setting_sys_time_blink_dir = !lcd_setting_sys_time_blink_dir;
	}


	if (TIME_UNIT_YEAR == lcd_setting_sys_time_unit)
	{
		if (0 == lcd_setting_sys_time_blink_dir)
		{
			lcd_clear_year(); // 需要先清除再写入数据，否则会有数据残留
			lcd_update_year(cur_setting_sys_time.year);
		}
		else
		{
			lcd_clear_year();
		}
	}
	else if (TIME_UNIT_MONTH == lcd_setting_sys_time_unit)
	{
		if (0 == lcd_setting_sys_time_blink_dir)
		{
			lcd_clear_month();
			lcd_update_month(cur_setting_sys_time.month);
		}
		else
		{
			lcd_clear_month();
		}

		// 月份闪烁，但是日期保持不变
		lcd_clear_day();
		lcd_update_day(cur_setting_sys_time.day);
	}
	else if (TIME_UNIT_DAY == lcd_setting_sys_time_unit)
	{
		if (0 == lcd_setting_sys_time_blink_dir)
		{
			lcd_clear_day();
			lcd_update_day(cur_setting_sys_time.day);
		}
		else
		{
			lcd_clear_day();
		}

		// 日期闪烁，但是月份保持不变
		lcd_clear_month();
		lcd_update_month(cur_setting_sys_time.month);
	}
	else if (TIME_UNIT_HOUR == lcd_setting_sys_time_unit)
	{
		if (0 == lcd_setting_sys_time_blink_dir)
		{
			lcd_clear_hour();
			lcd_update_hour(cur_setting_sys_time.hour);
		}
		else
		{
			lcd_clear_hour();
		}

		// 小时闪烁，但是分钟保持不变
		lcd_clear_min();
		lcd_update_min(cur_setting_sys_time.min);
	}
	else if (TIME_UNIT_MIN == lcd_setting_sys_time_unit)
	{
		if (0 == lcd_setting_sys_time_blink_dir)
		{
			lcd_clear_min();
			lcd_update_min(cur_setting_sys_time.min);
		}
		else
		{
			lcd_clear_min();
		}

		// 分钟闪烁，但是小时保持不变
		lcd_clear_hour();
		lcd_update_hour(cur_setting_sys_time.hour);
	}
	else if (TIME_UNIT_SEC == lcd_setting_sys_time_unit)
	{

	}
}

//  LCD屏显示处理  10ms执行一次
void lcdseg_handle(void)
{
#if 0

	if (sequencers.on_ff == DEVICE_ON)
	{

		blink_cnt++;

		update_cnt += 10;   //更新电压，功率显示 10ms

		if (blink_cnt == 30)  // 注意，这个变量在按键的单击处理也有使用的，修改==的条件，按键也需要修改
		{
			blink_cnt = 0;
			blink_f = !blink_f;
		}
		else if (lcd_now_state == open_dev_time)
		{
			// 设置开机延时
			// if (blink_f)
			// {
			// 	make_num(4, split_open_time[chose_relays_num][0]);
			// 	make_num(5, split_open_time[chose_relays_num][1]);
			// 	make_num(6, split_open_time[chose_relays_num][2]);
			// 	make_num(7, split_open_time[chose_relays_num][3]);

			// }
			// else
			// {
			// 	if (time_unit == 0)   // 分 十位
			// 	{
			// 		clean_num(4);

			// 	}
			// 	else if (time_unit == 1)  // 分 个位
			// 	{
			// 		clean_num(5);

			// 	}
			// 	else if (time_unit == 2) // 秒 十位
			// 	{
			// 		clean_num(6);

			// 	}
			// 	else if (time_unit == 3)  // 秒 个位
			// 	{
			// 		clean_num(7);

			// 	}
			// }

		}
		// 设置关机延时
		else if (lcd_now_state == close_dev_time)
		{
			// if (blink_f)
			// {
			// 	make_num(4, split_close_time[chose_relays_num][0]);
			// 	make_num(5, split_close_time[chose_relays_num][1]);
			// 	make_num(6, split_close_time[chose_relays_num][2]);
			// 	make_num(7, split_close_time[chose_relays_num][3]);

			// }
			// else
			// {
			// 	if (time_unit == 0)   // 分 十位
			// 	{
			// 		clean_num(4);

			// 	}
			// 	else if (time_unit == 1)  // 分 个位
			// 	{
			// 		clean_num(5);

			// 	}
			// 	else if (time_unit == 2) // 秒 十位
			// 	{
			// 		clean_num(6);

			// 	}
			// 	else if (time_unit == 3)  // 秒 个位
			// 	{
			// 		clean_num(7);

			// 	}
			// }
		}

		/* 调系统时间 */
		else if (lcd_now_state == set_sys_time)
		{
			if (blink_f)
			{
				clean_num(1);clean_num(2);clean_num(3);   //清
				clean_num(4);clean_num(5);clean_num(6); clean_num(7); // 清屏
				clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭 “V” "W" 

#if 0
				//显示年份
				if (sys_time_unit < 4)
				{
					clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4)); // 请 ' " 
					make_num(4, temp_year[0]);
					make_num(5, temp_year[1]);
					make_num(6, temp_year[2]);
					make_num(7, temp_year[3]);
				}
				else if (sys_time_unit >= 4 && sys_time_unit < 8)
				{
					//月
					make_num(4, temp_month[0]);
					make_num(5, temp_month[1]);
					//日
					make_num(6, temp_day[0]);
					make_num(7, temp_day[1]);

				}
				else
				{

					make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
					// 时
					make_num(2, temp_hour[0]);
					make_num(3, temp_hour[1]);
					// 分
					make_num(4, temp_min[0]);
					make_num(5, temp_min[1]);
					// 秒
					make_num(6, temp_sec[0]);
					make_num(7, temp_sec[1]);

				}
#endif

			}
			else
			{
#if 0
				if (sys_time_unit == 0)
				{
					clean_num(4);
				}
				else if (sys_time_unit == 1)
				{
					clean_num(5);

				}
				else if (sys_time_unit == 2)
				{
					clean_num(6);

				}
				else if (sys_time_unit == 3)
				{
					clean_num(7);

				}
				else if (sys_time_unit == 4)
				{
					clean_num(4);

				}
				else if (sys_time_unit == 5)
				{
					clean_num(5);

				}
				else if (sys_time_unit == 6)
				{
					clean_num(6);

				}
				else if (sys_time_unit == 7)
				{
					clean_num(7);

				}
				else if (sys_time_unit == 8)
				{
					clean_num(2);

				}
				else if (sys_time_unit == 9)
				{
					clean_num(3);

				}
				else if (sys_time_unit == 10)
				{
					clean_num(4);

				}
				else if (sys_time_unit == 11)
				{
					clean_num(5);

				}
				else if (sys_time_unit == 12)
				{
					clean_num(6);

				}
				else if (sys_time_unit == 13)
				{
					clean_num(7);

				}
#endif

			}


		}
		/* 调定时开继电器  */
		else if (lcd_now_state == timing_relay_open)
		{
			if (blink_f)
			{

				clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏
				clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
				// //显示年份
				// if (time_unit < 4)
				// {
				// 	clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4)); // 请 ' " 
				// 	make_num(4, set_countdown_open_year[chose_relays_num][0]);
				// 	make_num(5, set_countdown_open_year[chose_relays_num][1]);
				// 	make_num(6, set_countdown_open_year[chose_relays_num][2]);
				// 	make_num(7, set_countdown_open_year[chose_relays_num][3]);
				// }
				// else if (time_unit >= 4 && time_unit < 8)
				// {
				// 	//月
				// 	make_num(4, set_countdown_open_month[chose_relays_num][0]);
				// 	make_num(5, set_countdown_open_month[chose_relays_num][1]);
				// 	//日
				// 	make_num(6, set_countdown_open_day[chose_relays_num][0]);
				// 	make_num(7, set_countdown_open_day[chose_relays_num][1]);

				// }
				// else
				// {

				// 	make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
				// 	// 时
				// 	make_num(2, set_countdown_open_hour[chose_relays_num][0]);
				// 	make_num(3, set_countdown_open_hour[chose_relays_num][1]);
				// 	// 分
				// 	make_num(4, set_countdown_open_min[chose_relays_num][0]);
				// 	make_num(5, set_countdown_open_min[chose_relays_num][1]);
				// 	// 秒
				// 	make_num(6, set_countdown_open_sec[chose_relays_num][0]);
				// 	make_num(7, set_countdown_open_sec[chose_relays_num][1]);

				// }

			}
			else
			{
				// if (time_unit == 0)
				// {
				// 	clean_num(4);
				// }
				// else if (time_unit == 1)
				// {
				// 	clean_num(5);

				// }
				// else if (time_unit == 2)
				// {
				// 	clean_num(6);

				// }
				// else if (time_unit == 3)
				// {
				// 	clean_num(7);

				// }
				// else if (time_unit == 4)
				// {
				// 	clean_num(4);

				// }
				// else if (time_unit == 5)
				// {
				// 	clean_num(5);

				// }
				// else if (time_unit == 6)
				// {
				// 	clean_num(6);

				// }
				// else if (time_unit == 7)
				// {
				// 	clean_num(7);

				// }
				// else if (time_unit == 8)
				// {
				// 	clean_num(2);

				// }
				// else if (time_unit == 9)
				// {
				// 	clean_num(3);

				// }
				// else if (time_unit == 10)
				// {
				// 	clean_num(4);

				// }
				// else if (time_unit == 11)
				// {
				// 	clean_num(5);

				// }
				// else if (time_unit == 12)
				// {
				// 	clean_num(6);

				// }
				// else if (time_unit == 13)
				// {
				// 	clean_num(7);

				// }

			}

		}
		/* 调定时关继电器 */
		else if (lcd_now_state == timing_relay_close)
		{

			// if (blink_f)
			// {

			// 	clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏
			// 	clean_dis(clrbit(SEG_S5)); clean_dis(clrbit(SEG_S6)); // 关闭“V”"W""
			// 	//显示年份
			// 	if (time_unit < 4)
			// 	{
			// 		clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4)); // 请 ' " 
			// 		make_num(4, set_countdown_close_year[chose_relays_num][0]);
			// 		make_num(5, set_countdown_close_year[chose_relays_num][1]);
			// 		make_num(6, set_countdown_close_year[chose_relays_num][2]);
			// 		make_num(7, set_countdown_close_year[chose_relays_num][3]);
			// 	}
			// 	else if (time_unit >= 4 && time_unit < 8)
			// 	{
			// 		//月
			// 		make_num(4, set_countdown_close_month[chose_relays_num][0]);
			// 		make_num(5, set_countdown_close_month[chose_relays_num][1]);
			// 		//日
			// 		make_num(6, set_countdown_close_day[chose_relays_num][0]);
			// 		make_num(7, set_countdown_close_day[chose_relays_num][1]);

			// 	}
			// 	else
			// 	{

			// 		make_dis(SEG_S3);make_dis(SEG_S4);  // 显示 “ " ”  “ ' ”
			// 		// 时
			// 		make_num(2, set_countdown_close_hour[chose_relays_num][0]);
			// 		make_num(3, set_countdown_close_hour[chose_relays_num][1]);
			// 		// 分
			// 		make_num(4, set_countdown_close_min[chose_relays_num][0]);
			// 		make_num(5, set_countdown_close_min[chose_relays_num][1]);
			// 		// 秒
			// 		make_num(6, set_countdown_close_sec[chose_relays_num][0]);
			// 		make_num(7, set_countdown_close_sec[chose_relays_num][1]);

			// 	}

			// }
			// else
			// {
			// 	if (time_unit == 0)
			// 	{
			// 		clean_num(4);
			// 	}
			// 	else if (time_unit == 1)
			// 	{
			// 		clean_num(5);

			// 	}
			// 	else if (time_unit == 2)
			// 	{
			// 		clean_num(6);

			// 	}
			// 	else if (time_unit == 3)
			// 	{
			// 		clean_num(7);

			// 	}
			// 	else if (time_unit == 4)
			// 	{
			// 		clean_num(4);

			// 	}
			// 	else if (time_unit == 5)
			// 	{
			// 		clean_num(5);

			// 	}
			// 	else if (time_unit == 6)
			// 	{
			// 		clean_num(6);

			// 	}
			// 	else if (time_unit == 7)
			// 	{
			// 		clean_num(7);

			// 	}
			// 	else if (time_unit == 8)
			// 	{
			// 		clean_num(2);

			// 	}
			// 	else if (time_unit == 9)
			// 	{
			// 		clean_num(3);

			// 	}
			// 	else if (time_unit == 10)
			// 	{
			// 		clean_num(4);

			// 	}
			// 	else if (time_unit == 11)
			// 	{
			// 		clean_num(5);

			// 	}
			// 	else if (time_unit == 12)
			// 	{
			// 		clean_num(6);

			// 	}
			// 	else if (time_unit == 13)
			// 	{
			// 		clean_num(7);

			// 	}

			// }
		}



	}

	// if (BT_CONNECT_STATE)
	// {
	// 	make_dis(SEG_S1); // "IR"
	// }
	// else
	// {
	// 	clean_dis(clrbit(SEG_S1));
	// }

#endif

#if 0 // 测试显示屏的功能是否正常
	lcd_open_frame(); // 测试用 -- 打开背光，显示边框
	check_lcd_display(); // 测试用 
	lcd1621_write_data(dis_data, 16);
	// printf("test \n");
#endif

#if 1
	if (sequencer_status == SEQUENCER_STATUS_NONE)
	{
		// 上电期间，一直显示LCD，显示交流电电压：
		clean_num(1);clean_num(2);clean_num(3);   // 清显示数字的bit1 ~ bit3 
		clean_num(4);clean_num(5);clean_num(6); clean_num(7);  // 清屏
		clean_dis(clrbit(SEG_S3));clean_dis(clrbit(SEG_S4)); // 不显示： ' " 
		clean_dis(clrbit(SEG_S6)); // 关闭"W"
		// 交流电电压：
		make_num(1, voltage_array[0]);
		make_num(2, voltage_array[1]);
		make_num(3, voltage_array[2]);

		// USER_TO_DO 后面需要显示年份xx时间，再切换到显示月日xx时间，最后切换到显示时分
		user_sys_time_t user_sys_time;
		user_sys_time_get(&user_sys_time);
		lcd_update_year(user_sys_time.year);
	}
	else if (sequencer_status == SEQUENCER_STATUS_SETTING_SYS_TIME)
	{
		// clean_num(5);clean_num(6); clean_num(7);  // 清屏
		lcd_setting_sys_time_animation();
	}

#if 0
	{
		static u16 cnt = 0;
		cnt++;

		if (cnt < 100)
		{
			lcd_update_year(2014);
		}
		else if (cnt < 200)
		{
			lcd_clear_year();
		}
		else
		{
			cnt = 0;
		}
	}
#endif

	lcd1621_write_data(dis_data, 16); // 将lcd显示缓冲区的数据发送给屏幕驱动ic
	// lcd_1621_refresh();

#endif
}












#endif
