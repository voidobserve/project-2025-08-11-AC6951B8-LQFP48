#ifndef _LCD_USER_DRIVER_H_
#define _LCD_USER_DRIVER_H_

#define SEG_A           BIT(0)
#define SEG_B           BIT(1)
#define SEG_C           BIT(2)
#define SEG_D           BIT(3)
#define SEG_E           BIT(4)
#define SEG_F           BIT(5)
#define SEG_G           BIT(6)
#define SEG_H			BIT(7)
#define SEG_I           BIT(8)
#define SEG_J           BIT(9)
#define SEG_L           BIT(10)
#define SEG_M           BIT(11)
#define SEG_K           BIT(12)

#define LCDSEG_NC			0
#define LCDSEG_0        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_1        (SEG_B | SEG_C)
#define LCDSEG_2        (SEG_A | SEG_B | SEG_G | SEG_E | SEG_D)
#define LCDSEG_3        (SEG_A | SEG_B | SEG_G | SEG_C | SEG_D)
#define LCDSEG_4        (SEG_B | SEG_C | SEG_F | SEG_G)
#define LCDSEG_5        (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_6        (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_7        (SEG_A | SEG_B | SEG_C | SEG_F )
#define LCDSEG_8        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_9        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)

#define LCDSEG_A_C        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_B_C        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G| SEG_H)//(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)//
#define LCDSEG_C_C        (SEG_A | SEG_D | SEG_E | SEG_F)
#define LCDSEG_D_C        (SEG_A | SEG_B | SEG_C| SEG_D | SEG_H)//(SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)//
#define LCDSEG_E_C        (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_F_C        (SEG_A | SEG_E | SEG_F | SEG_G)
#define LCDSEG_G_C        (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_H_C        (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_I_C        (SEG_A | SEG_D |SEG_H)//(SEG_B | SEG_C)//
#define LCDSEG_J_C        (SEG_B | SEG_C | SEG_D | SEG_E)
#define LCDSEG_K_C        (SEG_E | SEG_F | SEG_G |SEG_J |SEG_M)//(SEG_E | SEG_F | SEG_G)//
#define LCDSEG_L_C        (SEG_D | SEG_E | SEG_F)
#define LCDSEG_M_C        (SEG_B | SEG_C | SEG_E |SEG_F |SEG_I |SEG_M)//(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//
#define LCDSEG_N_C        (SEG_B | SEG_C | SEG_E | SEG_F |SEG_I |SEG_J)//(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//
#define LCDSEG_O_C        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_P_C        (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define LCDSEG_Q_C        (SEG_A | SEG_B | SEG_C |SEG_D |SEG_E| SEG_F | SEG_J)//(SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)//
#define LCDSEG_R_C        (SEG_A | SEG_B | SEG_E | SEG_F |SEG_G |SEG_J)//(SEG_E | SEG_G)//
#define LCDSEG_S_C        (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_T_C        (SEG_A | SEG_H)//(SEG_D | SEG_E | SEG_F | SEG_G)//
#define LCDSEG_U_C        (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_V_C        (SEG_F | SEG_E |SEG_L | SEG_M)//(SEG_B | SEG_C | SEG_D | SEG_F | SEG_E)//
#define LCDSEG_W_C        (SEG_B | SEG_C | SEG_E |SEG_F |SEG_L |SEG_J)//(SEG_C | SEG_D | SEG_E)//
#define LCDSEG_X_C        (SEG_I | SEG_J | SEG_L | SEG_M)//(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)//
#define LCDSEG_Y_C        (SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_Z_C        (SEG_A | SEG_D | SEG_M | SEG_L)//(SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)//

#define LCDSEG_A        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_B        (SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)//(SEG_A | SEG_B | SEG_C | SEG_D | SEG_G| SEG_H)//
#define LCDSEG_C        (SEG_A | SEG_D | SEG_E | SEG_F)
#define LCDSEG_D        (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)//(SEG_A | SEG_B | SEG_C| SEG_D | SEG_H)//
#define LCDSEG_E        (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define LCDSEG_F        (SEG_A | SEG_E | SEG_F | SEG_G)
#define LCDSEG_G        (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_H        (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LCDSEG_I        (SEG_B | SEG_C)//(SEG_A | SEG_D |SEG_H)//
#define LCDSEG_J        (SEG_B | SEG_C | SEG_D | SEG_E)
#define LCDSEG_K        (SEG_E | SEG_F | SEG_G)//(SEG_E | SEG_F | SEG_G |SEG_J |SEG_M)//
#define LCDSEG_L        (SEG_D | SEG_E | SEG_F)
#define LCDSEG_M        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//(SEG_B | SEG_C | SEG_E |SEG_F |SEG_I |SEG_M)//
#define LCDSEG_N        (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)//(SEG_B | SEG_C | SEG_E | SEG_F |SEG_I |SEG_J)//
#define LCDSEG_O        (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_P        (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define LCDSEG_Q        (SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)//(SEG_A | SEG_B | SEG_C |SEG_D |SEG_E| SEG_F | SEG_J)//
#define LCDSEG_R        (SEG_E | SEG_G)//(SEG_A | SEG_B | SEG_E | SEG_F |SEG_G |SEG_J)//
#define LCDSEG_S        (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_T        (SEG_D | SEG_E | SEG_F | SEG_G)//(SEG_A | SEG_H)//
#define LCDSEG_U        (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define LCDSEG_V        (SEG_B | SEG_C | SEG_D | SEG_F | SEG_E)//(SEG_F | SEG_E |SEG_L | SEG_M)//
#define LCDSEG_W        (SEG_C | SEG_D | SEG_E)//(SEG_B | SEG_C | SEG_E |SEG_F |SEG_L |SEG_J)//
#define LCDSEG_X        (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)//(SEG_I | SEG_J | SEG_L | SEG_M)//
#define LCDSEG_Y        (SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define LCDSEG_Z        (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)//(SEG_A | SEG_D | SEG_M | SEG_L)//

#define LCDSEG_UP_     (SEG_A)
#define LCDSEG_DN_     (SEG_D)
#define LCDSEG_AMP     (SEG_A|SEG_D|SEG_I|SEG_J|SEG_M|SEG_L) // &
#define LCDSEG_APO     (SEG_M)  //'
#define LCDSEG_BRACKET_L   (SEG_M|SEG_J) //(
#define LCDSEG_BRACKET_R   (SEG_I|SEG_L) //)
#define LCDSEG_UNDEF    (SEG_G|SEG_H|SEG_I|SEG_J|SEG_M|SEG_L)//*
#define LCDSEG_PLUS	 	(SEG_G|SEG_H) //+
#define LCDSEG_COMMA   (SEG_L) //,
#define LCDSEG__           (SEG_G) //-
#define LCDSEG_POINT       (SEG_J) //.
#define LCDSEG_SLASH   (SEG_M|SEG_L)   // /
#define LCDSEG_1_MID   (SEG_H)   // |


typedef struct{
    u16 seg_buf[6];
    u16 flash_500ms;
    u32 rtc_time_ms;
    u8  pos_X;
    bool lock;
    u8  fft_enble;
    u16 fft_level;

    u16 screen_show_buf[8];

    u32 icon_buf[3];
    u32 flash_icon_buf[3];
    u16 flash_char_bit;
    u16 ignore_next_flash;  //忽略闪烁的下一次隐藏
    u16 pin_A[6];
    u16 pin_B[6];
    u16 pin_C[6];
    u16 anima_bef[8];
    u16 anima_count;
    u32 anima_msec;
    u16 raw_buf[9];         //原始缓存raw_buf, UI -> raw_buf+icon_buf+flash_icon_buf+flash_char_bit -(50ms:lock?return)> transfer_buf -> pin_ABC
    u32 transfer_buf[6];    //这个buffer对应6个COM的各个SEG：以该方式缓存：transfer_buf[COM1]=（BIT(SEG1)｜BIT(SEG2)),下一步再将其转为对应的COM和SEG
    u8  com_cnt;            //扫COM计数
}lcdseg_cb_t;
extern lcdseg_cb_t lcdseg_cb;

enum{
    COM1 = 0,
    COM2,
    COM3,
    COM4,
    COM5,
    COM6,
    COM_,
};

enum{
    SEG01 = 0,
    SEG02,
    SEG03,
    SEG04,
    SEG05,
    SEG06,
    SEG07,
    SEG08,
    SEG09,
    SEG10,
    SEG11,
    SEG12,
    SEG13,
    SEG14,
    SEG15,
    SEG16,
    SEG17,
    SEG18,
    SEG19,
    SEG20,
    SEG21,
    SEG22,
    SEG23,
    SEG24,
    SEG25,
    SEG26,
    SEG27,
    SEG28,
    SEG29,
    SEG30,
    SEG31,
    SEG32,
    SEG__,
};

#define LCDSEG_PLAY  	0 		//0: 播放图标
#define LCDSEG_PAUSE 	1 		//1: 暂停图标
#define LCDSEG_USB   	2 		//2: USB图标
#define LCDSEG_SD   	3 		//3: SD图标
#define LCDSEG_2POINT 	4 		//4: 冒号图标
#define LCDSEG_FM 		5 		//5: FM图标
#define LCDSEG_DOT 		6 		//6: 小数点图标
#define LCDSEG_MP3 		7 		//7: MP3图标
#define LCDSEG_REPEAT 	8 		//8: REPEAT图标
#define LCDSEG_CHARGE 	9 		//9: 充电图标
#define LCDSEG_BT 		10 	    //10: 充电图标
#define LCDSEG_AUX 		11  	//11: AUX图标
#define LCDSEG_WMA 		12 	    //12: WMA图标
#define LCDSEG_WAV 		13 	    //13: WAV图标
#define LCDSEG_L1 		14 	    //14
#define LCDSEG_L2 		15 	    //15
#define LCDSEG_L3 		16 	    //16
#define LCDSEG_L4 		17 	    //17
#define LCDSEG_MID_ 	18 	    //18: '-'图标
#define LCDSEG_INT 		19 	    //19: INT图标
#define LCDSEG_RPT 		20 	    //20: RPT图标
#define LCDSEG_RDM 		21 	    //21: RDM图标
#define LCDSEG_EQ 		22 	    //22: EQ图标
#define LCDSEG_POP 		23 	    //23: POP图标
#define LCDSEG_ROCK 	24 	    //24: ROCK图标
#define LCDSEG_CLASS 	25 	    //25: CLASS图标
#define LCDSEG_50KHz    26      //26: 5(50KHz)图标
#define LCDSEG_FLAT     27      //27: FLAT图标
#define LCDSEG_ST       28      //28: ST图标
#define LCDSEG_LOUD     29      //29: LOUD图标

#define LCDSEG_ICON_MAX 30

#endif
