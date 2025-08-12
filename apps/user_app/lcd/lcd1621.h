#ifndef _LCD1621_H_
#define _LCD1621_H_
extern u8 lcd1621_sendbuf[16];




typedef enum{
	
	show_power,     // ��ʾ�����͵�ѹ
	open_dev_time, // ��ʾ������ʱʱ��
	close_dev_time, // ��ʾ�ػ���ʱʱ��
	set_sys_time,  // ����ϵͳʱ��
	timing_relay_open, // ��ʱ���̵���
	timing_relay_close, // ��ʱ�ؼ̵���


}LCD_NODE;




// ����λ��seg���ĸ�com�����4��com
// ����λ���ڼ���seg�����16seg
typedef enum
{
	COM4 = 0,
	COM3,
	COM2,
	COM1,
}com_e; //�͵ײ��йأ��ײ��ȷ��͸�λ����˰�bit0��1��2��3˳�򵹹���

typedef enum
{
	SEG0 = 0,
	SEG1,
	SEG2,
	SEG3,
	SEG4,
	SEG5,
	SEG6,
	SEG7,
	SEG8,
	SEG9,
	SEG10,
	SEG11,
	SEG12,
	SEG13,  //
    SEG14,
    SEG15,
    SEG16,
}seg_e;

#define SEG_SET(com, seg)  ( ((1<<com)<<8) | seg)



#define COM_MASK  0x0F00
#define SEG_MASK  0x001F


#define SEG_1D SEG_SET(COM1,SEG0)	
#define SEG_1E SEG_SET(COM2,SEG0)	
#define SEG_1F SEG_SET(COM3,SEG0)	
#define SEG_1A SEG_SET(COM4,SEG0)	

#define SEG_S2 SEG_SET(COM1,SEG1)	
#define SEG_1C SEG_SET(COM2,SEG1)	
#define SEG_1G SEG_SET(COM3,SEG1)	    
#define SEG_1B SEG_SET(COM4,SEG1)	

#define SEG_2D SEG_SET(COM1,SEG2)	
#define SEG_2E SEG_SET(COM2,SEG2)	
#define SEG_2F SEG_SET(COM3,SEG2)	
#define SEG_2A SEG_SET(COM4,SEG2)	

#define SEG_S1 SEG_SET(COM1,SEG3)	
#define SEG_2C SEG_SET(COM2,SEG3)	
#define SEG_2G SEG_SET(COM3,SEG3)	
#define SEG_2B SEG_SET(COM4,SEG3)	

#define SEG_3D SEG_SET(COM1,SEG4)	
#define SEG_3E SEG_SET(COM2,SEG4)	
#define SEG_3F SEG_SET(COM3,SEG4)	
#define SEG_3A SEG_SET(COM4,SEG4)	

#define SEG_S5 SEG_SET(COM1,SEG5)	
#define SEG_3C SEG_SET(COM2,SEG5)	
#define SEG_3G SEG_SET(COM3,SEG5)	
#define SEG_3B SEG_SET(COM4,SEG5)	

#define SEG_X1 SEG_SET(COM1,SEG6)	
#define SEG_X2 SEG_SET(COM2,SEG6)	
#define SEG_X3 SEG_SET(COM3,SEG6)	
#define SEG_T1 SEG_SET(COM4,SEG6)	

#define SEG_4 SEG_SET(COM1,SEG7)	
#define SEG_3 SEG_SET(COM2,SEG7)	
#define SEG_2 SEG_SET(COM3,SEG7)	
#define SEG_1 SEG_SET(COM4,SEG7)	

#define SEG_8 SEG_SET(COM1,SEG8)	
#define SEG_7 SEG_SET(COM2,SEG8)	
#define SEG_6 SEG_SET(COM3,SEG8)	
#define SEG_5 SEG_SET(COM4,SEG8)	

#define SEG_7C SEG_SET(COM1,SEG9)	
#define SEG_7G SEG_SET(COM2,SEG9)	
#define SEG_7B SEG_SET(COM3,SEG9)	
#define SEG_S6 SEG_SET(COM4,SEG9)	

#define SEG_7D SEG_SET(COM1,SEG10)	
#define SEG_7E SEG_SET(COM2,SEG10)	
#define SEG_7F SEG_SET(COM3,SEG10)	
#define SEG_7A SEG_SET(COM4,SEG10)	

#define SEG_6C SEG_SET(COM1,SEG11)	
#define SEG_6G SEG_SET(COM2,SEG11)	
#define SEG_6B SEG_SET(COM3,SEG11)	
#define SEG_S4 SEG_SET(COM4,SEG11)	

#define SEG_6D SEG_SET(COM1,SEG12)	
#define SEG_6E SEG_SET(COM2,SEG12)	
#define SEG_6F SEG_SET(COM3,SEG12)	
#define SEG_6A SEG_SET(COM4,SEG12)	

#define SEG_5C SEG_SET(COM1,SEG13)	
#define SEG_5G SEG_SET(COM2,SEG13)	
#define SEG_5B SEG_SET(COM3,SEG13)	
#define SEG_S3 SEG_SET(COM4,SEG13)	

#define SEG_5D SEG_SET(COM1,SEG14)	
#define SEG_5E SEG_SET(COM2,SEG14)	
#define SEG_5F SEG_SET(COM3,SEG14)	
#define SEG_5A SEG_SET(COM4,SEG14)	

#define SEG_4C SEG_SET(COM1,SEG15)	
#define SEG_4G SEG_SET(COM2,SEG15)	
#define SEG_4B SEG_SET(COM3,SEG15)	
#define SEG_T SEG_SET(COM4,SEG15)	

#define SEG_4D SEG_SET(COM1,SEG16)
#define SEG_4E SEG_SET(COM2,SEG16)
#define SEG_4F SEG_SET(COM3,SEG16)
#define SEG_4A SEG_SET(COM4,SEG16)




void lcd1621_value_set(u8 *buff, u8 len);
void lcd1621_init(void);


void lcd1621_icon_update(void);
void lcd1621_flash_icon(void);

#endif
