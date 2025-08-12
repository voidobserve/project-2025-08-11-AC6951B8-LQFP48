#include "includes.h"
#include "app_config.h"
#include "app_task.h"
#include "app_main.h"
#include "asm/sdmmc.h"
#include "fm/fm_manage.h"

#include "ui/ui_api.h"
//#include "lcdseg.h"
#include "lcd_user_driver.h"
// #include "../../../../apps/soundbox/module/rgb_module.h"

#if USER_UI_LCDSEG_ENABLE

#define LCDSEG_DEBUG_ENABLE

#define LCDSEG_DEBUG_ENABLE
#ifdef LCDSEG_DEBUG_ENABLE
#define lcdseg_debug(fmt, ...) 	printf("[LCDSEG] "fmt, ##__VA_ARGS__)
#else
#define lcdseg_debug(...)
#endif

#define lcdseg_error(fmt, ...) 	printf("[LCDSEG ERR] "fmt, ##__VA_ARGS__)

/*
init
*/

#define LCDSEG_HW_EN            2                       // 硬驱选择: 0=软驱, 1=直驱, 2=1621

#define VOLTAGE_1_2             0                       // 1/2偏置电压
#define VOLTAGE_1_3             1                       // 模拟1/3偏置电压
#define VOLTAGE_1_3_LOWER       2                       // 比模拟1/3偏置电压更低，清屏更频繁

#define LCDSEG_VOLTAGE_SEL      VOLTAGE_1_3
#define LCDSEG_ANIMATION_TIME   14                      //Animation动画播放的总帧数

#define LCDSEG_PIN_NUM          36                      //LCDSEG屏幕PIN脚数量
#if LCDSEG_PIN_NUM == 17
    #define LCDSEG_SEG_OUT_LOW()    do{JL_PORTA->DIR &= ~0x1FFC; JL_PORTA->OUT &= ~0x1FFC;}while(0)
    #define LCDSEG_SEG_PORTA        0x1FFC
    #define LCDSEG_SEG_PORTC        0
    #define LCDSEG_SEG_PORTB        0
    #define LCDSEG_CHAR_NUM         4
#elif LCDSEG_PIN_NUM == 18
    #define LCDSEG_SEG_OUT_LOW()    do{JL_PORTA->DIR &= ~0x1FFE; JL_PORTA->OUT &= ~0x1FFE;}while(0)
    #define LCDSEG_SEG_PORTA        0x1FFE
    #define LCDSEG_SEG_PORTC        0
    #define LCDSEG_SEG_PORTB        0
    #define LCDSEG_CHAR_NUM         4
#elif LCDSEG_PIN_NUM == 20
    #define LCDSEG_SEG_OUT_LOW()    do{JL_PORTA->DIR &= ~0x1FFF; JL_PORTA->OUT &= ~0x1FFF; JL_PORTC->DIR &= ~0x0080; JL_PORTC->OUT &= ~0x0080;}while(0)
    #define LCDSEG_SEG_PORTA        0x1FFF
    #define LCDSEG_SEG_PORTC        0x0080
    #define LCDSEG_SEG_PORTB        0
    #define LCDSEG_CHAR_NUM         6
#elif LCDSEG_PIN_NUM == 22
    #define LCDSEG_SEG_OUT_LOW()    do{JL_PORTA->DIR &= ~0x1FFF; JL_PORTA->OUT &= ~0x1FFF; JL_PORTC->DIR &= ~0x0080; JL_PORTC->OUT &= ~0x0080; JL_PORTB->DIR &= ~0x0030; JL_PORTB->OUT &= ~0x0030;}while(0)
    #define LCDSEG_SEG_PORTA        0x1FFF
    #define LCDSEG_SEG_PORTC        0x0080
    #define LCDSEG_SEG_PORTB        0x0030
    #define LCDSEG_CHAR_NUM         7
#elif LCDSEG_PIN_NUM == 36
    #define LCDSEG_SEG_OUT_LOW()    do { } while(0)
    #define LCDSEG_SEG_PORTA        0
    #define LCDSEG_SEG_PORTC        0
    #define LCDSEG_SEG_PORTB        0
    #define LCDSEG_CHAR_NUM         9
#endif
#define LCDSEG_COM_IN_PUPD()    do{JL_PORTC->DIR |= 0x3F; JL_PORTC->PU |= 0x3F; JL_PORTC->PD |= 0x3F;}while(0)
#define LCDSEG_COM_OUT_HIGH(x)  do{JL_PORTC->PU &= ~x; JL_PORTC->PD &= ~x; JL_PORTC->DIR &= ~x; JL_PORTC->OUT |=  x;}while(0)
#define LCDSEG_COM_OUT_LOW(x)   do{JL_PORTC->PU &= ~x; JL_PORTC->PD &= ~x; JL_PORTC->DIR &= ~x; JL_PORTC->OUT &= ~x;}while(0)
#define LCDSEG_COMSEG_OUT_LOW() do{JL_PORTC->PU &= ~0x3F; JL_PORTC->PD &= ~0x3F; JL_PORTC->DIR &= ~0x3F; JL_PORTC->OUT &= ~0x3F; LCDSEG_SEG_OUT_LOW();}while(0)

static lcdseg_cb_t lcdseg_cb;

const u16 lcdseg_num_tbl[11] =
{
    LCDSEG_0, LCDSEG_1, LCDSEG_2, LCDSEG_3, LCDSEG_4,
    LCDSEG_5, LCDSEG_6, LCDSEG_7, LCDSEG_8, LCDSEG_9,
    LCDSEG_NC,
};

const u16 ID3_Table_Cap[] =
{
    LCDSEG_A_C,LCDSEG_B_C,LCDSEG_C_C,LCDSEG_D_C,LCDSEG_E_C,LCDSEG_F_C,LCDSEG_G_C,
    LCDSEG_H_C,LCDSEG_I_C,LCDSEG_J_C,LCDSEG_K_C,LCDSEG_L_C,LCDSEG_M_C,LCDSEG_N_C,
    LCDSEG_O_C,LCDSEG_P_C,LCDSEG_Q_C,LCDSEG_R_C,LCDSEG_S_C,LCDSEG_T_C,
    LCDSEG_U_C,LCDSEG_V_C,LCDSEG_W_C,LCDSEG_X_C,LCDSEG_Y_C,LCDSEG_Z_C,
};

const u16 ID3_Table[] =
{
    LCDSEG_A,LCDSEG_B,LCDSEG_C,LCDSEG_D,LCDSEG_E,LCDSEG_F,LCDSEG_G,
    LCDSEG_H,LCDSEG_I,LCDSEG_J,LCDSEG_K,LCDSEG_L,LCDSEG_M,LCDSEG_N,
    LCDSEG_O,LCDSEG_P,LCDSEG_Q,LCDSEG_R,LCDSEG_S,LCDSEG_T,
    LCDSEG_U,LCDSEG_V,LCDSEG_W,LCDSEG_X,LCDSEG_Y,LCDSEG_Z,
};

const u16 ID3_Symbol_Table[] =
{
    LCDSEG_AMP,LCDSEG_APO,LCDSEG_BRACKET_L,LCDSEG_BRACKET_R,LCDSEG_UNDEF,
    LCDSEG_PLUS,LCDSEG_COMMA,LCDSEG__,LCDSEG_POINT,LCDSEG_SLASH
};
//---------------------------------------------------------------------------------------------------------

static void lcdseg_clear(void)                          //将中转buf，图标和闪烁标志位都清零
{
    //memset(&lcdseg_cb.transfer_buf, 0, sizeof(lcdseg_cb.transfer_buf));//中转buf清零,保存6个COM的16(22PIN),14(20PIN),12(18PIN),11(17PIN)个SEG,预留作为保存1621驱动缓存
    memset(&lcdseg_cb.raw_buf, 0, sizeof(lcdseg_cb.raw_buf));
    // memset(&lcdseg_cb.anima_bef, 0, sizeof(lcdseg_cb.anima_bef));
    // lcdseg_cb.anima_count = 0;
    // lcdseg_cb.anima_msec = jiffies;
    lcdseg_cb.icon_buf[0] = 0;                          //图标常亮标志位
    lcdseg_cb.icon_buf[1] = 0;
    lcdseg_cb.icon_buf[2] = 0;
    lcdseg_cb.flash_icon_buf[0] = 0;                    //图标闪烁标志位
    lcdseg_cb.flash_icon_buf[1] = 0;
    lcdseg_cb.flash_icon_buf[2] = 0;
    lcdseg_cb.flash_char_bit = 0;                       //字符闪烁位，BIT(x)表示第x个字符需要闪烁
}

static void lcdseg_setXY(u32 x, u32 y)                  //定位屏幕上的字符位置
{
    lcdseg_cb.pos_X = x;
}

static void lcdseg_FlashChar(u32 x)                      //闪烁选定位的字符
{
    lcdseg_cb.flash_char_bit |= BIT(x);
}

static void lcdseg_Clear_FlashChar(u32 x)               //消除选定为的字符闪烁标志
{
    lcdseg_cb.flash_char_bit &= ~BIT(x);
}

static void lcdseg_show_icon(u32 x)                     //显示ICON图标
{
    if(x < 32)
        lcdseg_cb.icon_buf[0] |= BIT(x);
    else if(x < 64)
        lcdseg_cb.icon_buf[1] |= BIT(x-32);
    else //if(x<96)
        lcdseg_cb.icon_buf[2] |= BIT(x-64);
}

static void lcdseg_flash_icon(u32 x)                    //闪烁ICON图标
{
    if(x < 32)
        lcdseg_cb.flash_icon_buf[0] |= BIT(x);
    else if(x < 64)
        lcdseg_cb.flash_icon_buf[1] |= BIT(x-32);
    else //if(x<96)
        lcdseg_cb.flash_icon_buf[2] |= BIT(x-64);
}

static void lcdseg_clear_icon(u32 x)                    //清除ICON图标
{
    if(x < 32)
    {
        lcdseg_cb.icon_buf[0] &= ~BIT(x);
        lcdseg_cb.flash_icon_buf[0] &= ~BIT(x);
    }
    else if(x < 64)
    {
        lcdseg_cb.icon_buf[1] &= ~BIT(x-32);
        lcdseg_cb.flash_icon_buf[1] &= ~BIT(x-32);
    }
    else //if(x<96)
    {
        lcdseg_cb.icon_buf[2] &= ~BIT(x-64);
        lcdseg_cb.flash_icon_buf[2] &= ~BIT(x-64);
    }
}

static void lcdseg_show_string(u8* x)                   //在定位处开始显示字符串，自动进位
{
    u8 i = 0;
    u16 tmp = 0;
    while(x[i] != 0)
    {
        if((x[i]>37)&&(x[i]<48))                        //常见符号
            tmp = ID3_Symbol_Table[x[i]-38];
        else if((x[i]>47)&&(x[i]<58))                   //数字字符
            tmp = lcdseg_num_tbl[x[i]-48];
        else if((x[i]>64)&&(x[i]<91))                   //大写字母字符
            tmp = ID3_Table_Cap[x[i]-65];
        else if((x[i]>96)&&(x[i]<123))                  //小写字母字符
            tmp = ID3_Table[x[i]-97];
        else if(x[i]==95)
            tmp = LCDSEG__;
        else if(x[i]==32)
            tmp = 0;
        else if(x[i]==0)
            tmp = 0;
        else if(x[i]=='|')
            tmp = LCDSEG_1_MID;
        else
            tmp = LCDSEG_UNDEF;

        //lcdseg_cb.screen_show_buf[lcdseg_cb.pos_X + i] = tmp;
        lcdseg_cb.raw_buf[lcdseg_cb.pos_X + i] = tmp;
        i++;
        //lcdseg_disp_buff(tmp, lcdseg_cb.pos_X + i);
        //if((lcdseg_cb.pos_X + i) >= 7)
        //    break;
    }
}

static void lcdseg_record_anima_bef(u8* x)                   //在定位处开始显示字符串，自动进位
{
    // u8 i = 0;
    // u16 tmp = 0;
    // while(x[i] != 0)
    // {
    //     if((x[i]>37)&&(x[i]<48))                        //常见符号
    //         tmp = ID3_Symbol_Table[x[i]-38];
    //     else if((x[i]>47)&&(x[i]<58))                   //数字字符
    //         tmp = lcdseg_num_tbl[x[i]-48];
    //     else if((x[i]>64)&&(x[i]<91))                   //大写字母字符
    //         tmp = ID3_Table_Cap[x[i]-65];
    //     else if((x[i]>96)&&(x[i]<123))                  //小写字母字符
    //         tmp = ID3_Table[x[i]-97];
    //     else if(x[i]==95)
    //         tmp = LCDSEG__;
    //     else if(x[i]==32)
    //         tmp = 0;
    //     else if(x[i]==0)
    //         tmp = 0;
    //     else if(x[i]=='|')
    //         tmp = LCDSEG_1_MID;
    //     else
    //         tmp = LCDSEG_UNDEF;

    //     lcdseg_cb.anima_bef[lcdseg_cb.pos_X + i] = tmp;
    //     i++;
    // }
    for (u8 i = 0; i < 8; i++)
    {
        lcdseg_cb.anima_bef[i] = lcdseg_cb.raw_buf[i];
    }
    lcdseg_cb.anima_count = 0;
    lcdseg_cb.anima_msec = jiffies;
}

static void lcdseg_show_char(u8 x)                      //在定位处显示一个字符并自动进一位
{
    u16 tmp = 0;
    lcdseg_cb.pos_X++;
    if(x != 0)
    {
        if((x>37)&&(x<48))
            tmp = ID3_Symbol_Table[x-38];
        else if((x>47)&&(x<58))
            tmp = lcdseg_num_tbl[x-48];
        else if((x>64)&&(x<91))
            tmp = ID3_Table_Cap[x-65];
        else if((x>96)&&(x<123))
            tmp = ID3_Table[x-97];
        else if(x==95)
            tmp = LCDSEG__;
        else if(x==32)
            tmp = 0;
        else if(x==0)
            tmp = 0;
        else
            tmp = LCDSEG_UNDEF;

        //lcdseg_cb.screen_show_buf[lcdseg_cb.pos_X - 1] = tmp;
        lcdseg_cb.raw_buf[lcdseg_cb.pos_X - 1] = tmp;
        //lcdseg_disp_buff_hide(0, lcdseg_cb.pos_X);
        //lcdseg_disp_buff(tmp, lcdseg_cb.pos_X);
    }
}

static void lcdseg_lock(u8 lock)                        //锁定，禁止在锁定时：中转缓存buffer转化为屏幕扫描buffer
{
    lcdseg_cb.lock = lock;
}

static bool lcdseg_flash_500ms(void)                    //获取扫屏定时器所计算的500ms标志位
{
    return lcdseg_cb.flash_500ms;
}

static void lcdseg_ignore_next_flash(void)              //忽视下一次的闪烁，将其显示出来
{
    lcdseg_cb.ignore_next_flash = 1;
}

static void lcdseg_show_seg(u16 seg, u8 num)            //显示非预设字符，用于显示动画需要的非规则字符或图像
{
    //lcdseg_disp_buff(seg, num);
    lcdseg_cb.raw_buf[num - 1] = seg;
}

static LCD_API LCDSEG_HW = {
    .clear             = lcdseg_clear,                  //清除原始缓存raw_buffer
    .setXY             = lcdseg_setXY,                  //定位屏幕上的字符位置
    .FlashChar         = lcdseg_FlashChar,              //闪烁选定位的字符
    .Clear_FlashChar   = lcdseg_Clear_FlashChar,        //消除选定为的字符闪烁标志
    .show_icon         = lcdseg_show_icon,              //显示ICON图标
    .flash_icon        = lcdseg_flash_icon,             //闪烁ICON图标
    .clear_icon        = lcdseg_clear_icon,             //清除ICON图标
    .show_string       = lcdseg_show_string,            //在定位处开始显示字符串，自动进位
    .record_anima_bef  = lcdseg_record_anima_bef,
    .show_char         = lcdseg_show_char,              //在定位处显示一个字符并自动进一位
    //.show_number       = lcdseg_show_number,            //
    //.show_pic          = lcdseg_show_pic,               //
    //.hide_pic          = lcdseg_hide_pic,               //
    .lock              = lcdseg_lock,                   //锁定，禁止在锁定时：中转缓存buffer转化为屏幕扫描buffer
    .flash_500ms       = lcdseg_flash_500ms,            //获取扫屏定时器所计算的500ms标志位
    //.rtc_showtime_get  = lcdseg_rtc_showtime,           //
    .ignore_next_flash = lcdseg_ignore_next_flash,      //忽视下一次的闪烁，将其显示出来
    //.show_fft          = lcdseg_show_fft,               //
    //.show_fft_level    = lcdseg_show_fft_level,         //
    .show_seg          = lcdseg_show_seg,               //显示非预设字符，用于显示动画需要的非规则字符或图像
};

const u8 seg_2_pin_tbl[LCDSEG_CHAR_NUM*12][2] = {
#if LCDSEG_PIN_NUM == 20
    /*SEG_A*/   //SEG_A_COM+SEG_A_SEG
    {COM6,SEG02},{COM4,SEG03},{COM3,SEG03},{COM1,SEG03},{COM2,SEG02},{COM4,SEG02},{COM3,SEG02},{COM6,SEG03},{COM5,SEG02},{COM2,SEG03},{COM1,SEG02},{COM5,SEG03},
    {COM6,SEG04},{COM4,SEG05},{COM3,SEG05},{COM1,SEG05},{COM2,SEG04},{COM4,SEG04},{COM3,SEG04},{COM6,SEG05},{COM5,SEG04},{COM2,SEG05},{COM1,SEG04},{COM5,SEG05},
    {COM6,SEG06},{COM4,SEG07},{COM3,SEG07},{COM1,SEG07},{COM2,SEG06},{COM4,SEG06},{COM3,SEG06},{COM6,SEG07},{COM5,SEG06},{COM2,SEG07},{COM1,SEG06},{COM5,SEG07},
    {COM6,SEG08},{COM4,SEG09},{COM3,SEG09},{COM1,SEG09},{COM2,SEG08},{COM4,SEG08},{COM3,SEG08},{COM6,SEG09},{COM5,SEG08},{COM2,SEG09},{COM1,SEG08},{COM5,SEG09},
    {COM6,SEG10},{COM4,SEG11},{COM3,SEG11},{COM1,SEG11},{COM2,SEG10},{COM4,SEG10},{COM3,SEG10},{COM6,SEG11},{COM5,SEG10},{COM2,SEG11},{COM1,SEG10},{COM5,SEG11},
    {COM6,SEG13},{COM5,SEG13},{COM2,SEG13},{COM1,SEG14},{COM1,SEG13},{COM4,SEG13},{COM3,SEG13},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},
#elif LCDSEG_PIN_NUM == 22
    //SEG_A       SEG_B       SEG_C       SEG_D           SEG_E       SEG_F       SEG_G       SEG_H           SEG_I       SEG_J       SEG_L       SEG_M
    {COM6,SEG02},{COM6,SEG03},{COM4,SEG03},{COM3,SEG02},{COM4,SEG01},{COM6,SEG01},{COM4,SEG02},{COM5,SEG02},{COM5,SEG01},{COM3,SEG03},{COM3,SEG01},{COM5,SEG03},
    {COM6,SEG05},{COM6,SEG06},{COM4,SEG06},{COM3,SEG05},{COM4,SEG04},{COM6,SEG04},{COM4,SEG05},{COM5,SEG05},{COM5,SEG04},{COM3,SEG06},{COM3,SEG04},{COM5,SEG06},
    {COM6,SEG08},{COM6,SEG09},{COM4,SEG09},{COM3,SEG08},{COM4,SEG07},{COM6,SEG07},{COM4,SEG08},{COM5,SEG08},{COM5,SEG07},{COM3,SEG09},{COM3,SEG07},{COM5,SEG09},
    {COM6,SEG11},{COM6,SEG12},{COM4,SEG12},{COM3,SEG11},{COM4,SEG10},{COM6,SEG10},{COM4,SEG11},{COM5,SEG11},{COM5,SEG10},{COM3,SEG12},{COM3,SEG10},{COM5,SEG12},
    {COM6,SEG14},{COM6,SEG15},{COM4,SEG15},{COM3,SEG14},{COM4,SEG13},{COM6,SEG13},{COM4,SEG14},{COM5,SEG14},{COM5,SEG13},{COM3,SEG15},{COM3,SEG13},{COM5,SEG15},
    {COM2,SEG15},{COM1,SEG12},{COM1,SEG13},{COM1,SEG14},{COM2,SEG14},{COM2,SEG12},{COM2,SEG13},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},
    {COM1,SEG16},{COM1,SEG01},{COM1,SEG02},{COM1,SEG03},{COM2,SEG03},{COM2,SEG01},{COM2,SEG02},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},
#elif LCDSEG_PIN_NUM == 36
    //SEG_A       SEG_B       SEG_C       SEG_D           SEG_E       SEG_F       SEG_G       SEG_H           SEG_I       SEG_J       SEG_L       SEG_M
    {COM4,SEG04},{COM4,SEG05},{COM2,SEG05},{COM1,SEG04},{COM2,SEG03},{COM4,SEG03},{COM2,SEG04},{COM3,SEG04},{COM3,SEG03},{COM1,SEG05},{COM1,SEG03},{COM3,SEG05},
    {COM4,SEG07},{COM4,SEG08},{COM2,SEG08},{COM1,SEG07},{COM2,SEG06},{COM4,SEG06},{COM2,SEG07},{COM3,SEG07},{COM3,SEG06},{COM1,SEG08},{COM1,SEG06},{COM3,SEG08},
    {COM4,SEG10},{COM4,SEG11},{COM2,SEG11},{COM1,SEG10},{COM2,SEG09},{COM4,SEG09},{COM2,SEG10},{COM3,SEG10},{COM3,SEG09},{COM1,SEG11},{COM1,SEG09},{COM3,SEG11},
    {COM4,SEG14},{COM4,SEG15},{COM2,SEG15},{COM1,SEG14},{COM2,SEG13},{COM4,SEG13},{COM2,SEG14},{COM3,SEG14},{COM3,SEG13},{COM1,SEG15},{COM1,SEG13},{COM3,SEG15},
    {COM4,SEG17},{COM4,SEG18},{COM2,SEG18},{COM1,SEG17},{COM2,SEG16},{COM4,SEG16},{COM2,SEG17},{COM3,SEG17},{COM3,SEG16},{COM1,SEG18},{COM1,SEG16},{COM3,SEG18},
    {COM4,SEG20},{COM4,SEG21},{COM2,SEG21},{COM1,SEG20},{COM2,SEG19},{COM4,SEG19},{COM2,SEG20},{COM3,SEG20},{COM3,SEG19},{COM1,SEG21},{COM1,SEG19},{COM3,SEG21},
    {COM4,SEG25},{COM4,SEG26},{COM2,SEG26},{COM1,SEG25},{COM2,SEG24},{COM4,SEG24},{COM2,SEG25},{COM3,SEG25},{COM3,SEG24},{COM1,SEG26},{COM1,SEG24},{COM3,SEG26},
    {COM4,SEG28},{COM4,SEG29},{COM2,SEG29},{COM1,SEG28},{COM2,SEG27},{COM4,SEG27},{COM2,SEG28},{COM3,SEG28},{COM3,SEG27},{COM1,SEG29},{COM1,SEG27},{COM3,SEG29},
    {COM4,SEG31},{COM3,SEG31},{COM2,SEG31},{COM1,SEG30},{COM2,SEG30},{COM4,SEG30},{COM3,SEG30},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},{COM_,SEG__},
#endif
};
static void lcdseg_char_transfer(u8 show_flag)          //将字符转换到中转缓存,在这里根据不同真值表的显示屏来转换
{
    u8 seg_cnt = 0, char_num = 0;
    u8 tmp;
    // if (jiffies > lcdseg_cb.anima_msec + msecs_to_jiffies(50))
    {
        // lcdseg_cb.anima_msec = jiffies;
        if (lcdseg_cb.anima_count < 0xFFFF)
            lcdseg_cb.anima_count++;
    }
    for(char_num = 0; char_num < LCDSEG_CHAR_NUM; char_num++)
    {
        if((lcdseg_cb.flash_char_bit & BIT(char_num)) && !show_flag)     //隐藏标志
            continue;
        for(seg_cnt = 0; seg_cnt < 12; seg_cnt++)       //SEG_A ~ SEG_M
        {
#if 1 // old
            if (lcdseg_cb.raw_buf[char_num] & BIT(seg_cnt))
            {
                tmp = char_num*12+seg_cnt;
                if(seg_2_pin_tbl[tmp][0] == COM_)
                    continue;
                lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
            }
#elif 0
            if (lcdseg_cb.anima_bef[char_num] & BIT(seg_cnt))
            {
                tmp = char_num * 12 + seg_cnt;
                if (seg_2_pin_tbl[tmp][0] != COM_)
                {
                    switch (lcdseg_cb.anima_count)
                    {
                    default:  // 不显示
                        break;
                    case 6:  // 6: 不显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_H, SEG_G, SEG_F, SEG_B, SEG_I, SEG_M, SEG_A
                        if (BIT(seg_cnt) == SEG_A)
                            break;
                    case 5:  // 5: 不显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_H, SEG_G, SEG_F, SEG_B, SEG_I, SEG_M
                        if (BIT(seg_cnt) == SEG_F || BIT(seg_cnt) == SEG_B || BIT(seg_cnt) == SEG_I || BIT(seg_cnt) == SEG_M)
                            break;
                    case 4:  // 4: 不显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_H, SEG_G
                        if (BIT(seg_cnt) == SEG_G)
                            break;
                    case 3:  // 3: 不显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_H
                        if (BIT(seg_cnt) == SEG_E || BIT(seg_cnt) == SEG_C || BIT(seg_cnt) == SEG_L || BIT(seg_cnt) == SEG_J || BIT(seg_cnt) == SEG_H)
                            break;
                    case 2:  // 2: 不显示SEG_D
                        if (BIT(seg_cnt) == SEG_D)
                            break;
                    case 1:  // 第一帧, 完全显示转换前的文本
                    case 0:  // 第一帧, 完全显示转换前的文本
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    }
                }
            }
            if (lcdseg_cb.raw_buf[char_num] & BIT(seg_cnt))
            {
                tmp = char_num * 12 + seg_cnt;
                if (seg_2_pin_tbl[tmp][0] != COM_)
                {
                    switch (lcdseg_cb.anima_count)
                    {
                    case 0:  // 第一帧, 完全不显示转换前的文本
                    case 1:  // 第一帧, 完全不显示转换前的文本
                    case 2:  // 第一帧, 完全不显示转换前的文本
                        // lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 3:  // 2: 显示SEG_D
                        if (BIT(seg_cnt) == SEG_D)
                            lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 4:  // 3: 显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J
                        if (BIT(seg_cnt) == SEG_D || BIT(seg_cnt) == SEG_E || BIT(seg_cnt) == SEG_C || BIT(seg_cnt) == SEG_L || BIT(seg_cnt) == SEG_J)
                            lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 5:  // 4: 显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_G
                        if (BIT(seg_cnt) == SEG_D || BIT(seg_cnt) == SEG_E || BIT(seg_cnt) == SEG_C || BIT(seg_cnt) == SEG_L || BIT(seg_cnt) == SEG_J || BIT(seg_cnt) == SEG_G)
                            lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 6:  // 5: 显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_H, SEG_G, SEG_F, SEG_B, SEG_I, SEG_M
                        if (BIT(seg_cnt) == SEG_D || BIT(seg_cnt) == SEG_E || BIT(seg_cnt) == SEG_C || BIT(seg_cnt) == SEG_L || BIT(seg_cnt) == SEG_J || BIT(seg_cnt) == SEG_H || BIT(seg_cnt) == SEG_G || BIT(seg_cnt) == SEG_F || BIT(seg_cnt) == SEG_B || BIT(seg_cnt) == SEG_I || BIT(seg_cnt) == SEG_M)
                            lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 7:  // 6: 显示SEG_D, SEG_E, SEG_C, SEG_L, SEG_J, SEG_H, SEG_G, SEG_F, SEG_B, SEG_I, SEG_M, SEG_A
                        if (BIT(seg_cnt) == SEG_D || BIT(seg_cnt) == SEG_E || BIT(seg_cnt) == SEG_C || BIT(seg_cnt) == SEG_L || BIT(seg_cnt) == SEG_J || BIT(seg_cnt) == SEG_H || BIT(seg_cnt) == SEG_G || BIT(seg_cnt) == SEG_F || BIT(seg_cnt) == SEG_B || BIT(seg_cnt) == SEG_I || BIT(seg_cnt) == SEG_M || BIT(seg_cnt) == SEG_A)
                            lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    default:  // 全显示
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    }
                }
            }
#elif 0
            if (lcdseg_cb.anima_bef[char_num] & BIT(seg_cnt))
            {
                tmp = char_num * 12 + seg_cnt;
                if (seg_2_pin_tbl[tmp][0] != COM_)
                {
                    switch (lcdseg_cb.anima_count)
                    {
                    default:  // 不显示
                        break;
                    case 3:
                        if (0) {}
                        else if (BIT(seg_cnt) == SEG_D) tmp = tmp - seg_cnt + 0;
                        else break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 2:
                        if (0) {}
                        else if (BIT(seg_cnt) == SEG_C) tmp = tmp - seg_cnt + 1;
                        else if (BIT(seg_cnt) == SEG_D) tmp = tmp - seg_cnt + 6;
                        else if (BIT(seg_cnt) == SEG_G) tmp = tmp - seg_cnt + 0;
                        else if (BIT(seg_cnt) == SEG_J) tmp = tmp - seg_cnt + 11;
                        else if (BIT(seg_cnt) == SEG_L) tmp = tmp - seg_cnt + 8;
                        else break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 1:  // 第一帧, 完全显示转换前的文本
                    case 0:  // 第一帧, 完全显示转换前的文本
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    }
                }
            }
            if (lcdseg_cb.raw_buf[char_num] & BIT(seg_cnt))
            {
                tmp = char_num * 12 + seg_cnt;
                if (seg_2_pin_tbl[tmp][0] != COM_)
                {
                    switch (lcdseg_cb.anima_count)
                    {
                    case 0:  // 第一帧, 完全不显示转换前的文本
                    case 1:  // 第一帧, 完全不显示转换前的文本
                        break;
                    case 2:
                        if (0) {}
                        else if (BIT(seg_cnt) == SEG_A) tmp = tmp - seg_cnt + 3;
                        else break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    case 3:
                        if (0) {}
                        else if (BIT(seg_cnt) == SEG_A) tmp = tmp - seg_cnt + 6;
                        else if (BIT(seg_cnt) == SEG_B) tmp = tmp - seg_cnt + 2;
                        else if (BIT(seg_cnt) == SEG_F) tmp = tmp - seg_cnt + 4;
                        else if (BIT(seg_cnt) == SEG_I) tmp = tmp - seg_cnt + 10;
                        else if (BIT(seg_cnt) == SEG_M) tmp = tmp - seg_cnt + 9;
                        else break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                    default:  // 全显示
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    }
                }
            }
#elif 1
            if (lcdseg_cb.anima_bef[char_num] & BIT(seg_cnt))
            {
                tmp = char_num * 12 + seg_cnt;
                if (seg_2_pin_tbl[tmp][0] != COM_)
                {
                    switch (lcdseg_cb.anima_count)
                    {
                    default:  // 不显示
                        break;
                    case 6:
                        if (char_num <= 4) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp - 12 * 5][0]] |= BIT(seg_2_pin_tbl[tmp - 12 * 5][1]);
                        break;
                    case 5:
                        if (char_num <= 3) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp - 12 * 4][0]] |= BIT(seg_2_pin_tbl[tmp - 12 * 4][1]);
                        break;
                    case 4:
                        if (char_num <= 2) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp - 12 * 3][0]] |= BIT(seg_2_pin_tbl[tmp - 12 * 3][1]);
                        break;
                    case 3:
                        if (char_num <= 1) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp - 12 * 2][0]] |= BIT(seg_2_pin_tbl[tmp - 12 * 2][1]);
                        break;
                    case 2:
                        if (char_num <= 0) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp - 12 * 1][0]] |= BIT(seg_2_pin_tbl[tmp - 12 * 1][1]);
                        break;
                    case 1:  // 第一帧, 完全显示转换前的文本
                    case 0:  // 第一帧, 完全显示转换前的文本
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    }
                }
            }
            if (lcdseg_cb.raw_buf[char_num] & BIT(seg_cnt))
            {
                tmp = char_num * 12 + seg_cnt;
                if (seg_2_pin_tbl[tmp][0] != COM_)
                {
                    switch (lcdseg_cb.anima_count)
                    {
                    case 0:  // 第一帧, 完全不显示转换前的文本
                    case 1:  // 第一帧, 完全不显示转换前的文本
                    case 2:  // 第一帧, 完全不显示转换前的文本
                        break;
                    case 3:
                        if (char_num >= 1) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp + 12 * 5][0]] |= BIT(seg_2_pin_tbl[tmp + 12 * 5][1]);
                        break;
                    case 4:
                        if (char_num >= 2) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp + 12 * 4][0]] |= BIT(seg_2_pin_tbl[tmp + 12 * 4][1]);
                        break;
                    case 5:
                        if (char_num >= 3) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp + 12 * 3][0]] |= BIT(seg_2_pin_tbl[tmp + 12 * 3][1]);
                        break;
                    case 6:
                        if (char_num >= 2) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp + 12 * 2][0]] |= BIT(seg_2_pin_tbl[tmp + 12 * 2][1]);
                        break;
                    case 7:
                        if (char_num >= 1) break;
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp + 12 * 1][0]] |= BIT(seg_2_pin_tbl[tmp + 12 * 1][1]);
                        break;
                    default:  // 全显示
                        lcdseg_cb.transfer_buf[seg_2_pin_tbl[tmp][0]] |= BIT(seg_2_pin_tbl[tmp][1]);
                        break;
                    }
                }
            }
#endif
        }
    }
}

#if LCDSEG_PIN_NUM == 20
#define COM1SEG01_ICON  0xFF                            //当图标里有位于{COM1,SEG01}的，则填上该图标，如LCDSEG_2POINT，否则填上0xFF
const u8 icon_2_pin_tbl[LCDSEG_ICON_MAX][2] = {         //按照lcd_user_driver.h里的ICON顺序排列
    /*LCDSEG_PLAY*/
    [LCDSEG_2POINT] = {COM2,SEG12},
    [LCDSEG_DOT]    = {COM1,SEG12},
    [LCDSEG_INT]    = {COM5,SEG12},
    [LCDSEG_RPT]    = {COM4,SEG12},
    [LCDSEG_RDM]    = {COM3,SEG12},
    [LCDSEG_POP]    = {COM6,SEG01},
    [LCDSEG_ROCK]   = {COM4,SEG01},
    [LCDSEG_FLAT]   = {COM3,SEG01},
    [LCDSEG_CLASS]  = {COM5,SEG01},
    [LCDSEG_50KHz]  = {COM6,SEG14},
};
const u8 fft_2_pin_tbl[4][2] = {                        //FFT:L1,L2,L3,L4
    {COM5,SEG14},{COM4,SEG14},{COM3,SEG14},{COM2,SEG14}
};
#elif LCDSEG_PIN_NUM == 22
#define COM1SEG01_ICON  0xFF                            //当图标里有位于{COM1,SEG01}的，则填上该图标，如LCDSEG_2POINT，否则填上0xFF
const u8 icon_2_pin_tbl[LCDSEG_ICON_MAX][2] = {         //按照lcd_user_driver.h里的ICON顺序排列
    [LCDSEG_2POINT] = {COM2,SEG11},
    [LCDSEG_DOT]    = {COM1,SEG15},
    [LCDSEG_USB]    = {COM2,SEG07},
    [LCDSEG_SD]     = {COM1,SEG08},
    [LCDSEG_MID_]   = {COM2,SEG10},
    [LCDSEG_INT]    = {COM4,SEG16},
    [LCDSEG_RPT]    = {COM5,SEG16},
    [LCDSEG_RDM]    = {COM6,SEG16},
    [LCDSEG_EQ]     = {COM1,SEG10},
    [LCDSEG_POP]    = {COM1,SEG06},
    [LCDSEG_ROCK]   = {COM2,SEG06},
    [LCDSEG_CLASS]  = {COM1,SEG11},
    [LCDSEG_BT]     = {COM1,SEG07},
};
const u8 fft_2_pin_tbl[4][2] = {                        //FFT:L1,L2,L3,L4
    {COM2,SEG05},{COM1,SEG05},{COM1,SEG04},{COM2,SEG04}
};
#elif LCDSEG_PIN_NUM == 36
#define COM1SEG01_ICON  LCDSEG_USB                      //当图标里有位于{COM1,SEG01}的，则填上该图标，如LCDSEG_2POINT，否则填上0xFF
const u8 icon_2_pin_tbl[LCDSEG_ICON_MAX][2] = {         //按照lcd_user_driver.h里的ICON顺序排列
    [LCDSEG_2POINT] = {COM1,SEG31},
    [LCDSEG_DOT]    = {COM1,SEG31},
    [LCDSEG_USB]    = {COM1,SEG01},
    [LCDSEG_SD]     = {COM1,SEG22},
    [LCDSEG_INT]    = {COM4,SEG12},
    [LCDSEG_RPT]    = {COM3,SEG22},
    [LCDSEG_RDM]    = {COM4,SEG22},
    // [LCDSEG_EQ]     = {COM3,SEG23},
    [LCDSEG_POP]    = {COM4,SEG23},
    [LCDSEG_ROCK]   = {COM1,SEG12},
    [LCDSEG_CLASS]  = {COM2,SEG12},
    // [LCDSEG_FLAT]   = {COM2,SEG22},
    [LCDSEG_ST]     = {COM1,SEG23},
    [LCDSEG_LOUD]   = {COM2,SEG23},
};
#define FFT_SEG_NUMBER  5
const u8 fft_2_pin_tbl[FFT_SEG_NUMBER][2] = {           //FFT:L1,L2,L3,L4,L5
    {COM4,SEG32},{COM3,SEG32},{COM2,SEG32},{COM1,SEG32},{COM1,SEG02}
};
#define CD_SEG_DISPLAY
const u8 cd_2_pin_tbl[][2] = { {COM4,SEG02}, {COM3,SEG02}, {COM2,SEG02} };
#endif
static void lcdseg_icon_transfer(u8 show_flag)          //将图标转换到中转缓存 & 图标闪烁处理
{
    u8 icon_num = 0;
    u8 icon_num_fix = 0;
    for(icon_num = 0; icon_num < LCDSEG_ICON_MAX; icon_num++)
    {
        if(icon_num < 32)
            icon_num_fix = 0;
        else if(icon_num < 64)
            icon_num_fix = 1;
        else //if(icon_num <96)
            icon_num_fix = 2;

        if((lcdseg_cb.flash_icon_buf[icon_num_fix] & BIT(icon_num-icon_num_fix*32)) && !show_flag)     //隐藏标志
            continue;
        if(lcdseg_cb.icon_buf[icon_num_fix] & BIT(icon_num-icon_num_fix*32) || (lcdseg_cb.flash_icon_buf[icon_num_fix] & BIT(icon_num-icon_num_fix*32)))
        {
#if COM1SEG01_ICON != 0xFF                              //当图标里有{COM1,SEG01}就另设判断
            if(icon_2_pin_tbl[icon_num][0] == COM1 && icon_2_pin_tbl[icon_num][1] == SEG01 && icon_num != COM1SEG01_ICON)
                continue;
#else
            if(icon_2_pin_tbl[icon_num][0] == COM1 && icon_2_pin_tbl[icon_num][1] == SEG01)
                continue;
#endif
            lcdseg_cb.transfer_buf[icon_2_pin_tbl[icon_num][0]] |= BIT(icon_2_pin_tbl[icon_num][1]);
        }
    }
}

static void lcdseg_fft_transfer(void)                   //生成FFT频谱转换到中转缓存
{
    extern u8 User_mute_flag;
    extern s8 User_volume_value;
    extern int dac_energy_level;
    static u8 fft_wave_cnt = 0;
    if(dac_energy_level >= 1 && User_mute_flag == 0 && User_volume_value)
    {
        fft_wave_cnt++;
        if(fft_wave_cnt >= 5)
            dac_energy_level = 4;
        lcdseg_cb.transfer_buf[fft_2_pin_tbl[0][0]] |= BIT(fft_2_pin_tbl[0][1]);
        if(dac_energy_level >= 4)
        {
            fft_wave_cnt = 0;
            lcdseg_cb.transfer_buf[fft_2_pin_tbl[1][0]] |= BIT(fft_2_pin_tbl[1][1]);
            if(dac_energy_level >= 8)
            {
                lcdseg_cb.transfer_buf[fft_2_pin_tbl[2][0]] |= BIT(fft_2_pin_tbl[2][1]);
                if(dac_energy_level >= 11)
                {
                    lcdseg_cb.transfer_buf[fft_2_pin_tbl[3][0]] |= BIT(fft_2_pin_tbl[3][1]);
#if FFT_SEG_NUMBER == 5
                    if (dac_energy_level >= 13)
                    {
                        lcdseg_cb.transfer_buf[fft_2_pin_tbl[4][0]] |= BIT(fft_2_pin_tbl[4][1]);
                    }
#endif
                }
            }
        }
    }
#ifdef CD_SEG_DISPLAY
#define CD_COUNT_TIME 7
    static u8 round_count = 0;
    if (!app_check_curr_task(APP_POWEROFF_TASK) && User_mute_flag == 0 && User_volume_value)
    {
        // if ((round_count / CD_COUNT_TIME) == 0)
        //     lcdseg_cb.transfer_buf[cd_2_pin_tbl[0][0]] |= BIT(cd_2_pin_tbl[0][1]);
        // else if ((round_count / CD_COUNT_TIME) == 1)
        //     lcdseg_cb.transfer_buf[cd_2_pin_tbl[1][0]] |= BIT(cd_2_pin_tbl[1][1]);
        // else if ((round_count / CD_COUNT_TIME) == 2)
        //     lcdseg_cb.transfer_buf[cd_2_pin_tbl[2][0]] |= BIT(cd_2_pin_tbl[2][1]);
        // round_count++;
        // if ((round_count / CD_COUNT_TIME) >= 3)
        //     round_count = 0;

            
        lcdseg_cb.transfer_buf[cd_2_pin_tbl[0][0]] |= BIT(cd_2_pin_tbl[0][1]);
        lcdseg_cb.transfer_buf[cd_2_pin_tbl[1][0]] |= BIT(cd_2_pin_tbl[1][1]);
        lcdseg_cb.transfer_buf[cd_2_pin_tbl[2][0]] |= BIT(cd_2_pin_tbl[2][1]);
        if ((round_count / CD_COUNT_TIME) == 0)
            lcdseg_cb.transfer_buf[cd_2_pin_tbl[0][0]] &= ~BIT(cd_2_pin_tbl[0][1]);
        else if ((round_count / CD_COUNT_TIME) == 1)
            lcdseg_cb.transfer_buf[cd_2_pin_tbl[1][0]] &= ~BIT(cd_2_pin_tbl[1][1]);
        else if ((round_count / CD_COUNT_TIME) == 2)
            lcdseg_cb.transfer_buf[cd_2_pin_tbl[2][0]] &= ~BIT(cd_2_pin_tbl[2][1]);
        round_count++;
        if ((round_count / CD_COUNT_TIME) >= 3)
            round_count = 0;
    }
#endif
}

static void transfer_2_pin(void)                        //将中转缓存转换到最终缓存
{
#if USER_UI_LCDSEG_ENABLE
    u8 com = 0;
    for(;com<6;com++)
    {
        lcdseg_cb.pin_A[com] = 0;
        lcdseg_cb.pin_B[com] = 0;
        lcdseg_cb.pin_C[com] = 0;
        if(lcdseg_cb.transfer_buf[com] & BIT(0))     lcdseg_cb.pin_A[com] |= BIT(12);
        if(lcdseg_cb.transfer_buf[com] & BIT(1))     lcdseg_cb.pin_A[com] |= BIT(11);
        if(lcdseg_cb.transfer_buf[com] & BIT(2))     lcdseg_cb.pin_A[com] |= BIT(10);
        if(lcdseg_cb.transfer_buf[com] & BIT(3))     lcdseg_cb.pin_A[com] |= BIT(9);
        if(lcdseg_cb.transfer_buf[com] & BIT(4))     lcdseg_cb.pin_A[com] |= BIT(8);
        if(lcdseg_cb.transfer_buf[com] & BIT(5))     lcdseg_cb.pin_A[com] |= BIT(7);
        if(lcdseg_cb.transfer_buf[com] & BIT(6))     lcdseg_cb.pin_A[com] |= BIT(6);
        if(lcdseg_cb.transfer_buf[com] & BIT(7))     lcdseg_cb.pin_A[com] |= BIT(5);
        if(lcdseg_cb.transfer_buf[com] & BIT(8))     lcdseg_cb.pin_A[com] |= BIT(4);
        if(lcdseg_cb.transfer_buf[com] & BIT(9))     lcdseg_cb.pin_A[com] |= BIT(3);
        if(lcdseg_cb.transfer_buf[com] & BIT(10))    lcdseg_cb.pin_A[com] |= BIT(2);
        if(lcdseg_cb.transfer_buf[com] & BIT(11))    lcdseg_cb.pin_A[com] |= BIT(1);
        if(lcdseg_cb.transfer_buf[com] & BIT(12))    lcdseg_cb.pin_A[com] |= BIT(0);
        if(lcdseg_cb.transfer_buf[com] & BIT(13))    lcdseg_cb.pin_C[com] |= BIT(7);
        if(lcdseg_cb.transfer_buf[com] & BIT(14))    lcdseg_cb.pin_B[com] |= BIT(5);
        if(lcdseg_cb.transfer_buf[com] & BIT(15))    lcdseg_cb.pin_B[com] |= BIT(4);
    }
#elif USER_UI_1621LCD_ENABLE
    //将中转缓存转换为1621数据并发送出去
#endif
}

static void lcdseg_buff_transfer(void)                  //50ms将从UI里获取的原始显示数据raw_buf转换为中转缓存transfer_buf进行闪烁等处理，然后转换为扫屏缓存pin_ABC
{
    if(lcdseg_cb.lock == 0)                             //正在从UI处获取raw_buf
    {
        //获取现在的闪烁标志位(0:隐藏;1:显示)和闪烁忽略标志位(1:强制显示)，确认当前是否处于闪烁的隐藏状态
        u8 show_flag = lcdseg_cb.flash_500ms | lcdseg_cb.ignore_next_flash;     //0:闪烁隐藏; 1:正常显示
        if(lcdseg_cb.ignore_next_flash == 1 && !lcdseg_cb.flash_500ms)
            lcdseg_cb.ignore_next_flash = 2;
        else if(lcdseg_cb.ignore_next_flash == 2 && lcdseg_cb.flash_500ms)
            lcdseg_cb.ignore_next_flash = 0;
        //先将中转缓存清零
        memset(&lcdseg_cb.transfer_buf, 0, sizeof(lcdseg_cb.transfer_buf));
#if LCDSEG_HW_EN == 2
        extern u8 lcd1621_sendbuf[16];
        memset(lcd1621_sendbuf, 0, sizeof(lcd1621_sendbuf));
#endif
        //将字符转换到中转缓存 & 字符闪烁处理
        lcdseg_char_transfer(show_flag);
        //将图标转换到中转缓存 & 图标闪烁处理
        lcdseg_icon_transfer(show_flag);
        //生成FFT频谱转换到中转缓存
        lcdseg_fft_transfer();
        //将中转缓存转换到最终缓存
#if LCDSEG_HW_EN == 1
        extern void lcdseg_set_data(u8 com, u16 seg);
        for (u8 i = 0; i < 6; i++)
        {
            lcdseg_set_data(i, lcdseg_cb.transfer_buf[i]);
        }
#elif LCDSEG_HW_EN == 2
        //将中转缓存转换为1621数据并发送出去
        extern void lcd1621_value_set(u8 *buff, u8 len);
        
        for (u8 ht1621_seg = 0; ht1621_seg < (LCDSEG_PIN_NUM - 4) / 2; ht1621_seg++)
        {
            if (lcdseg_cb.transfer_buf[0] & BIT(ht1621_seg * 2 + 1)) lcd1621_sendbuf[ht1621_seg] |= BIT(0);
            if (lcdseg_cb.transfer_buf[1] & BIT(ht1621_seg * 2 + 1)) lcd1621_sendbuf[ht1621_seg] |= BIT(1);
            if (lcdseg_cb.transfer_buf[2] & BIT(ht1621_seg * 2 + 1)) lcd1621_sendbuf[ht1621_seg] |= BIT(2);
            if (lcdseg_cb.transfer_buf[3] & BIT(ht1621_seg * 2 + 1)) lcd1621_sendbuf[ht1621_seg] |= BIT(3);
            if (lcdseg_cb.transfer_buf[0] & BIT(ht1621_seg * 2 + 0)) lcd1621_sendbuf[ht1621_seg] |= BIT(4);
            if (lcdseg_cb.transfer_buf[1] & BIT(ht1621_seg * 2 + 0)) lcd1621_sendbuf[ht1621_seg] |= BIT(5);
            if (lcdseg_cb.transfer_buf[2] & BIT(ht1621_seg * 2 + 0)) lcd1621_sendbuf[ht1621_seg] |= BIT(6);
            if (lcdseg_cb.transfer_buf[3] & BIT(ht1621_seg * 2 + 0)) lcd1621_sendbuf[ht1621_seg] |= BIT(7);
        }
        lcd1621_value_set(lcd1621_sendbuf, (LCDSEG_PIN_NUM - 4) / 2);
#else
        transfer_2_pin();
#endif
    }
}

//LCDSEG复用检测时的SEG_IO控制
AT_VOLATILE_RAM_CODE
static u8 lcdseg_reuse_io_set(u32 detect_io, u8 detect_mode)              //复用检测时:detect_io为要控制的IO口, detect_mode为检测模式
{
    JL_PORT_FLASH_TypeDef *jl_port = NULL;
    if(detect_io <= IO_PORTA_15)
        jl_port = JL_PORTA;
    else if(detect_io <= IO_PORTC_15)
        jl_port = JL_PORTC;
    else if(detect_io <= IO_PORTB_15)
        jl_port = JL_PORTB;

    if(jl_port)
    {
        if(detect_mode == 0)            //输入上拉
        {
            jl_port->DIE |= BIT(detect_io);
            jl_port->DIR |= BIT(detect_io);
            jl_port->PU  |= BIT(detect_io);
        }
        else if(detect_mode == 1)       //延时
        {
            for(u16 i=30;i>0;i--)
            {
                asm("nop");
            }
        }
        else if(detect_mode == 2)       //检测电平并返回值
        {
            if(jl_port->IN & BIT(detect_io))
                return 1;               //返回电平值
            else
                return 0;
        }
        else if(detect_mode == 3)       //输出低，关上拉
        {
            jl_port->DIE &= ~BIT(detect_io);
            jl_port->DIR &= ~BIT(detect_io);
            jl_port->PU  &= ~BIT(detect_io);
        }
    }
    return 2;                           //当前不是电平值
}

extern const struct sdmmc_platform_data *sd_data_for_det;
u8 sd_online = 0;
//LCDSEG复用检测SD_DET
int User_sdmmc_0_io_detect(const struct sdmmc_platform_data *sd_data)   //250Ms底层会自动调用进入检测一次
{
#if 0
    app_var.io_det_sd0_working = 1;

    lcdseg_reuse_io_set(sd_data->detect_io, 0);
    lcdseg_reuse_io_set(sd_data->detect_io, 1);

    sd_online = lcdseg_reuse_io_set(sd_data->detect_io, 2);
    if(sd_data->detect_io_level == 0)   //检测低电平
    {
        sd_online ^= 1;                 //低电平有效时需反转
    }

    lcdseg_reuse_io_set(sd_data->detect_io, 3);

    app_var.io_det_sd0_working = 0;
#endif
    return sd_online;
}

//u8 io_encoder_sta = 0;
//AT_VOLATILE_RAM_CODE
//static void lcdseg_reuse_io(void)
//{
//#if 1
//#if (TCFG_SD0_DET_MODE == SD_IO_DECT)
//    lcdseg_reuse_io_set(sd_data->detect_io, 0);
//#endif
//#if USER_IO_KNOB_ENABLE
//    lcdseg_reuse_io_set(, 0);
//#endif
//
//    lcdseg_reuse_io_set(0, 1);         //延时让上拉电阻把IO拉高
//
//#if (TCFG_SD0_DET_MODE == SD_IO_DECT)
//    sd_online = lcdseg_reuse_io_set(sd_data->detect_io, 2);
//    if(sd_data->detect_io_level == 0)   //检测低电平
//    {
//        sd_online ^= 1;                 //低电平有效时需反转
//    }
//#endif
//#if USER_IO_KNOB_ENABLE
//
//#endif
//
//    lcdseg_reuse_io_set(sd_data->detect_io, 3);
//#endif
//}

u8 knob_direction = 0;
AT_VOLATILE_RAM_CODE
static void knob_detect(void)
{
    lcdseg_cb.rtc_time_ms++;
    if(lcdseg_cb.rtc_time_ms > 86400000)   ///1day = 24*60*60*1000ms
        lcdseg_cb.rtc_time_ms = 0;

    u16 flash_500ms = lcdseg_cb.rtc_time_ms % 1000;
	if(flash_500ms == 500)
        lcdseg_cb.flash_500ms = 1;
    else if(flash_500ms == 0/*1000*/)
        lcdseg_cb.flash_500ms = 0;

	u8 state = 0;
	static u8  old_state = 0,knob_scan_data = 0;
#if ACC_DETECT_AD_REUSE_KNOB
    u8 acc_in = 0;
#endif

#if USER_AD_KNOB_ENABLE || USER_IO_KNOB_ENABLE
    extern u32 adc_get_knob_value(u32 ch);
    u16 ad_knob_val = adc_get_knob_value(USER_AD_KNOB_AD_CHANNEL);

    static u16 sys_reset_cnt = 0;
    if(ad_knob_val<255){
        if(sys_reset_cnt <= 1000)
            sys_reset_cnt++;
        if(sys_reset_cnt == 250 && knob_direction<=2) knob_direction = 3;
        return;
    }
    sys_reset_cnt = 0;
#if USER_AD_KNOB_ENABLE
	if(ad_knob_val<340/*352*/)         state = 0x03;           // 33K//18K//8.2K = 4.8K
	else if(ad_knob_val<384/*389*/)    state = 0x02;           // 18K//8.2K      = 5.6K
	else if(ad_knob_val<438/*436*/)    state = 0x01;           // 33K//8.2K      = 6.6K
#if ACC_DETECT_AD_REUSE_KNOB
	else if(ad_knob_val>500)    acc_in = 1;             //[(33K//18K):∞]    = [11.6K(MIN):∞(MAX)]
    User_update_acc_det(acc_in);
#endif
#elif USER_IO_KNOB_ENABLE
#if ACC_DETECT_AD_REUSE_KNOB
    if(ad_knob_val>500)         acc_in = 1;             //[(33K//18K):∞]    = [11.6K(MIN):∞(MAX)]
    User_update_acc_det(acc_in);
#endif
#endif
#endif
	if(state != old_state)
	{
		old_state = state;
		knob_scan_data |= state;
		knob_scan_data<<=2;
#if CONFIG_CLIENT_VK                                    //维科使用的是12脉冲
        knob_scan_data &= 0x3f;
		if((knob_scan_data==0x10)||(knob_scan_data==0x2C))
#else                                                   //默认使用的是24脉冲
		if(knob_scan_data==0xB4)
#endif
		{
			knob_direction= 1;
		}
#if CONFIG_CLIENT_VK
		else if((knob_scan_data==0x20)||(knob_scan_data==0x1C))
#else
		else if(knob_scan_data==0x78)
#endif
		{
			knob_direction= 2;
		}
	}
}

AT_VOLATILE_RAM_CODE
static void lcdseg_scan(void)
{
//    lcdseg_reuse_io();
    //knob&acc 1ms 扫描处理
#if !TCFG_UART0_ENABLE
    knob_detect();
#endif

    if(app_var.rgb_reuse_working)                       //RGB3线控制MCU工作时不进行扫屏操作
        return;
    if(app_var.io_det_sd0_working)                      //LCDSEG复用检测SD工作时不进行扫屏操作
        return;

#if LCDSEG_HW_EN >= 1
    return;
#endif

    //开始扫描：显示的内容从最终port缓存buffer提取 portA[COMx]&BIT(SEGx)...
    //设置需要显示的COM 输出高，SEG 输出低，下1ms周期相反，循环6次，共12周期(12ms)
#if LCDSEG_VOLTAGE_SEL == VOLTAGE_1_2
    if(++lcdseg_cb.com_cnt >= 12)
        lcdseg_cb.com_cnt = 0;
    u8 dir = (lcdseg_cb.com_cnt + 1) % 2;
    u8 com = lcdseg_cb.com_cnt / 2;
    LCDSEG_COM_IN_PUPD();                               //COM 输入开上下拉
    LCDSEG_SEG_OUT_LOW();                               //SEG 输出低电平
    if(dir)
    {
        LCDSEG_COM_OUT_LOW(BIT(5-com));

        JL_PORTA->OUT |= lcdseg_cb.pin_A[com]&LCDSEG_SEG_PORTA;
        JL_PORTC->OUT |= lcdseg_cb.pin_C[com]&LCDSEG_SEG_PORTC;
        JL_PORTB->OUT |= lcdseg_cb.pin_B[com]&LCDSEG_SEG_PORTB;
    }
    else
    {
        LCDSEG_COM_OUT_HIGH(BIT(5-com));

        JL_PORTA->OUT |= (~lcdseg_cb.pin_A[com])&LCDSEG_SEG_PORTA;
        JL_PORTC->OUT |= (~lcdseg_cb.pin_C[com])&LCDSEG_SEG_PORTC;
        JL_PORTB->OUT |= (~lcdseg_cb.pin_B[com])&LCDSEG_SEG_PORTB;
    }
#else
    static u8 cnt = 0;
    cnt++;
#if LCDSEG_VOLTAGE_SEL == VOLTAGE_1_3
    if(cnt <= 2)                                       //<=2:正反一次循环才清一次屏,模拟1/3偏置电压
#elif LCDSEG_VOLTAGE_SEL == VOLTAGE_1_3_LOWER
    if(cnt <= 1)                                       //<=1:正反都清屏,比模拟1/3偏置电压的电压还低
#endif
    {
        if(++lcdseg_cb.com_cnt >= 12)
            lcdseg_cb.com_cnt = 0;
        u8 dir = (lcdseg_cb.com_cnt + 1) % 2;
        u8 com = lcdseg_cb.com_cnt / 2;
        LCDSEG_COM_IN_PUPD();                           //COM 输入开上下拉
        LCDSEG_SEG_OUT_LOW();                           //SEG 输出低电平
        if(dir)
        {
            LCDSEG_COM_OUT_LOW(BIT(5-com));

            JL_PORTA->OUT |= lcdseg_cb.pin_A[com]&LCDSEG_SEG_PORTA;
            JL_PORTC->OUT |= lcdseg_cb.pin_C[com]&LCDSEG_SEG_PORTC;
            JL_PORTB->OUT |= lcdseg_cb.pin_B[com]&LCDSEG_SEG_PORTB;
        }
        else
        {
            LCDSEG_COM_OUT_HIGH(BIT(5-com));

            JL_PORTA->OUT |= (~lcdseg_cb.pin_A[com])&LCDSEG_SEG_PORTA;
            JL_PORTC->OUT |= (~lcdseg_cb.pin_C[com])&LCDSEG_SEG_PORTC;
            JL_PORTB->OUT |= (~lcdseg_cb.pin_B[com])&LCDSEG_SEG_PORTB;
        }
    }
    else
    {
        cnt = 0;
        LCDSEG_COMSEG_OUT_LOW();                        //COM、SEG均输出低
    }
#endif
}

void *lcdseg_init(const struct led7_platform_data * _data)//lcdseg初始化,IO,设置定时器
{
    lcdseg_debug("Dong:lcdseg init DDDDDDDDDDDDDDDDDDDD\n");
    lcdseg_debug("%s", __func__);

    memset(&lcdseg_cb, 0, sizeof(lcdseg_cb));

#if USER_UI_LCDSEG_ENABLE

    user_rgb_online();///检测RGB是否在线
#if LCDSEG_HW_EN
#if LCDSEG_HW_EN == 1
    extern void lcdseg_hw_init(void);
    lcdseg_hw_init();
#elif LCDSEG_HW_EN == 2
    extern void lcd1621_init(void);
    lcd1621_init();
#endif

    void app_timer_led_scan(void (*led_scan)(void *));
    lcdseg_buff_transfer();
    sys_timer_add(NULL, lcdseg_buff_transfer, 50);
    app_timer_led_scan(lcdseg_scan);                    //将扫屏设置为1ms的定时器Timer2
    return (&LCDSEG_HW);
#endif

    u16 port_A = 0, port_B = 0, port_C = 0x3F;
    port_A |= LCDSEG_SEG_PORTA;
    port_C |= LCDSEG_SEG_PORTC;
    port_B |= LCDSEG_SEG_PORTB;

    JL_PORTA->DIE  &= ~port_A;  JL_PORTC->DIE  &= ~port_C;  JL_PORTB->DIE  &= ~port_B;
    JL_PORTA->DIEH &= ~port_A;  JL_PORTC->DIEH &= ~port_C;  JL_PORTB->DIEH &= ~port_B;
    JL_PORTA->DIR  &= ~port_A;  JL_PORTC->DIR  &= ~port_C;  JL_PORTB->DIR  &= ~port_B;
    JL_PORTA->PU   &= ~port_A;  JL_PORTC->PU   &= ~port_C;  JL_PORTB->PU   &= ~port_B;
    JL_PORTA->PD   &= ~port_A;  JL_PORTC->PD   &= ~port_C;  JL_PORTB->PD   &= ~port_B;
    JL_PORTA->HD   &= ~port_A;  JL_PORTC->HD   &= ~port_C;  JL_PORTB->HD   &= ~port_B;
    JL_PORTA->HD0  &= ~port_A;  JL_PORTC->HD0  &= ~port_C;  JL_PORTB->HD0  &= ~port_B;
    JL_PORTA->OUT  &= ~port_A;  JL_PORTC->OUT  &= ~port_C;  JL_PORTB->OUT  &= ~port_B;

    lcdseg_buff_transfer();
    sys_timer_add(NULL, lcdseg_buff_transfer, 50);

    void app_timer_led_scan(void (*led_scan)(void *));
    app_timer_led_scan(lcdseg_scan);                    //将扫屏设置为1ms的定时器Timer2
#elif USER_UI_1621LCD_ENABLE
    lcd1621_init();
    sys_timeout_add(NULL, lcd1621_updata, 50);   ///2020-12-10 需要实现50Ms定时传输1621数据

//    sys_timeout_add(NULL, rf_detect, 500);

    void app_timer_led_scan(void (*led_scan)(void *));
    app_timer_led_scan(knob_detect);
//    app_timer_led_scan(timer125us_hook);
#endif
    return (&LCDSEG_HW);
}

const u16 lcdseg_anima_tbl0[14][4] =
{
    {(SEG_D),(0),(0),(0)},
    {(SEG_D),(SEG_D),(0),(0)},
    {(SEG_D),(SEG_D),(SEG_D),(0)},
    {(SEG_D),(SEG_D),(SEG_D),(SEG_D)},
    {(0),(SEG_D),(SEG_D),(SEG_D|SEG_C)},
    {(0),(0),(SEG_D),(SEG_D|SEG_C|SEG_B)},
    {(0),(0),(0),(SEG_D|SEG_C|SEG_B|SEG_A)},
    {(0),(0),(SEG_A),(SEG_C|SEG_B|SEG_A)},
    {(0),(SEG_A),(SEG_A),(SEG_B|SEG_A)},
    {(SEG_A),(SEG_A),(SEG_A),(SEG_A)},
    {(SEG_A|SEG_F),(SEG_A),(SEG_A),(0)},
    {(SEG_A|SEG_F|SEG_E),(SEG_A),(0),(0)},

    {1,1,0,0},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl1[14][4] =
{
    {(0),(SEG_D),(SEG_D),(0)},
    {(SEG_D),(SEG_D),(SEG_D),(SEG_D)},
    {(SEG_D|SEG_E),(SEG_D),(SEG_D),(SEG_C|SEG_D)},
    {(SEG_D|SEG_E|SEG_G),(SEG_D),(SEG_D),(SEG_C|SEG_D|SEG_G)},
    {(SEG_D|SEG_E|SEG_G),(SEG_D|SEG_G),(SEG_D|SEG_G),(SEG_C|SEG_D|SEG_G)},
    {(SEG_D|SEG_E|SEG_G),(SEG_B|SEG_D|SEG_G),(SEG_D|SEG_F|SEG_G),(SEG_C|SEG_D|SEG_G)},
    {(SEG_D|SEG_E|SEG_G),(SEG_A|SEG_B|SEG_D|SEG_G),(SEG_A|SEG_D|SEG_F|SEG_G),(SEG_C|SEG_D|SEG_G)},
    {(SEG_A|SEG_D|SEG_E|SEG_G),(SEG_A|SEG_B|SEG_D|SEG_G),(SEG_A|SEG_D|SEG_F|SEG_G),(SEG_A|SEG_C|SEG_D|SEG_G)},
    {(SEG_E|SEG_F),(0),(0),(SEG_B|SEG_C)},
    {(SEG_B|SEG_C),(0),(0),(SEG_E|SEG_F)},
    {(0),(SEG_E|SEG_F),(SEG_B|SEG_C),(0)},
    {(0),(SEG_B|SEG_C),(SEG_E|SEG_F),(0)},

    {0,1,1,0},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl2[14][4] =
{
    {(0),(SEG_D),(SEG_D),(0)},
    {(SEG_D),(SEG_D),(SEG_D),(SEG_D)},
    {(SEG_D|SEG_E),(SEG_D),(SEG_D),(SEG_C|SEG_D)},
    {(SEG_D|SEG_E|SEG_F),(SEG_D),(SEG_D),(SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_D),(SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_B|SEG_D|SEG_E|SEG_F),(0),(0),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_F)},
    {(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),(0),(0),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F)},
    {(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F)},
    {(0),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),(0)},
    {(0),(SEG_A|SEG_B|SEG_C|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_E|SEG_F),(0)},
    {(0),(SEG_A|SEG_B|SEG_C),(SEG_A|SEG_E|SEG_F),(0)},
    {(0),(SEG_B|SEG_C),(SEG_E|SEG_F),(0)},

    {0,1,1,0},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl3[14][4] =
{
    {(SEG_E),(0),(0),(SEG_B)},
    {(SEG_E|SEG_F),(0),(0),(SEG_B|SEG_C)},
    {(SEG_A|SEG_E|SEG_F),(0),(0),(SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_E|SEG_F),(SEG_A),(SEG_D),(SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_E|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_D)},
    {(SEG_D|SEG_E|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_C)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_D),(SEG_A),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A),(SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_E|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_C|SEG_D)},

    {1,0,0,1},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl4[14][4] =
{
    {(0),(SEG_A),(SEG_A),(0)},
    {(SEG_A),(SEG_A),(SEG_A),(SEG_A)},
    {(SEG_A|SEG_F),(SEG_A),(SEG_A),(SEG_A|SEG_B)},
    {(SEG_A|SEG_E|SEG_F),(SEG_A),(SEG_A),(SEG_A|SEG_B|SEG_C)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A),(SEG_A),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_D),(SEG_A|SEG_D|SEG_E),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D),(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(0),(SEG_B|SEG_C),(SEG_E|SEG_F),(0)},
    {(0),(SEG_E|SEG_F),(SEG_B|SEG_C),(0)},
    {(SEG_B|SEG_C),(0),(0),(SEG_E|SEG_F)},
    {(SEG_E|SEG_F),(0),(0),(SEG_B|SEG_C)},

    {0,1,1,0},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl5[14][4] =
{
    {(0),(0),(SEG_A),(SEG_A)},
    {(0),(0),(SEG_A),(SEG_A|SEG_B|SEG_C)},
    {(0),(0),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_D),(SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_D|SEG_E|SEG_F),(SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D),(SEG_D),(SEG_B|SEG_C|SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D),(SEG_D),(SEG_D)},
    {(SEG_A|SEG_D|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C|SEG_D),(0),(0)},
    {(SEG_A|SEG_E|SEG_F),(SEG_A|SEG_B|SEG_C),(0),(0)},
    {(SEG_A),(SEG_A|SEG_B|SEG_C),(0),(0)},
    {(0),(SEG_B|SEG_C),(0),(0)},

    {0,1,1,0},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl6[14][4] =
{
    {(SEG_D),(0),(0),(SEG_A)},
    {(SEG_D),(SEG_D),(SEG_A),(SEG_A)},
    {(SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A)},
    {(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_A|SEG_D)},
    {(SEG_A),(SEG_A|SEG_D),(SEG_A|SEG_D),(SEG_D)},
    {(SEG_A),(SEG_A),(SEG_D),(SEG_D)},
    {(SEG_A),(0),(0),(SEG_D)},
    {(SEG_E|SEG_F),(0),(0),(SEG_B|SEG_C)},
    {(SEG_B|SEG_C),(0),(0),(SEG_E|SEG_F)},
    {(0),(SEG_E|SEG_F),(SEG_B|SEG_C),(0)},
    {(0),(SEG_B|SEG_C),(SEG_E|SEG_F),(0)},
    {(0),(SEG_C),(SEG_F),(0)},

    {0,1,1,0},
    {1,1,1,1},
};
const u16 lcdseg_anima_tbl7[14][4] =
{
    {(SEG_E),(0),(0),(SEG_B)},
    {(SEG_E|SEG_G),(0),(0),(SEG_B|SEG_G)},
    {(SEG_B|SEG_E|SEG_G),(0),(0),(SEG_B|SEG_E|SEG_G)},
    {(SEG_B|SEG_E|SEG_G),(SEG_A),(SEG_D),(SEG_B|SEG_E|SEG_G)},
    {(SEG_B|SEG_E|SEG_G),(SEG_A|SEG_B),(SEG_D|SEG_E),(SEG_B|SEG_E|SEG_G)},
    {(SEG_B|SEG_E|SEG_G),(SEG_A|SEG_B|SEG_C),(SEG_D|SEG_E|SEG_F),(SEG_B|SEG_E|SEG_G)},
    {(SEG_B|SEG_G),(SEG_A|SEG_B|SEG_C),(SEG_D|SEG_E|SEG_F),(SEG_E|SEG_G)},
    {(SEG_B),(SEG_A|SEG_B|SEG_C),(SEG_D|SEG_E|SEG_F),(SEG_E)},
    {(0),(SEG_A|SEG_B|SEG_C),(SEG_D|SEG_E|SEG_F),(0)},
    {(0),(SEG_B|SEG_C),(SEG_E|SEG_F),(0)},
    {(0),(SEG_C),(SEG_F),(0)},
    {(0),(0),(0),(0)},

    {0,1,1,0},
    {1,1,1,1},
};

u8 lcdseg_anima_step = 0;
u8 lcdseg_anima_task = 0;
u8 lcdseg_anima_num = 0;
u16 anima_table[14][4] = {0};

const char *lcdseg_animation_task_tbl[] =
{
#if CONFIG_CLIENT_VK
    " ",
    "HELLO",
    "BYEBYE",
    "  BT",
    " DEV",
    "RADIO",
    " REC",
    " AUX",
    " RTC",
    "  PC",
    "SPDIF",
    " IDLE",
#else
    " ",
    " ",
    "BYE",
    " BT",
    "DEV",
    "RAD",
    "REC",
    "AUX",
    "RTC",
    " PC",
    "SPDI",
    "IDLE",
#endif
};

static void __ui_show_animation_finish(void)
{
//    UI_SHOW_MENU(MENU_BLOCK, 10, 0, NULL);
    lcdseg_anima_step = LCDSEG_ANIMATION_TIME + 2;
    ui_set_auto_reflash(500);
    UI_REFLASH_WINDOW(1);
}
#if 0//CONFIG_CLIENT_VK
static void __ui_show_animation_sec(void)
{
    lcdseg_anima_task = app_get_curr_task();
    UI_SHOW_MENU(MENU_ANIMA, 1000, lcdseg_anima_step, __ui_show_animation_finish);
}
#endif
static void __ui_show_animation(void)
{
    if(User_acc_sta >= ACC_EVENT_ENTER)
        return;
#if 0//CONFIG_CLIENT_VK
    lcdseg_anima_step = LCDSEG_ANIMATION_TIME;
    if(lcdseg_anima_task == APP_POWERON_TASK)
        UI_SHOW_MENU(MENU_ANIMA, 1000, lcdseg_anima_step, __ui_show_animation_sec);
    else
        UI_SHOW_MENU(MENU_ANIMA, 1000, lcdseg_anima_step, __ui_show_animation_sec);
#else
    lcdseg_anima_step++;
    if(lcdseg_anima_step > LCDSEG_ANIMATION_TIME)
    {
        UI_SHOW_MENU(MENU_ANIMA, 1000, lcdseg_anima_step-1, __ui_show_animation_finish);
        return;
    }
    else if(lcdseg_anima_step > 11)
    {
        UI_SHOW_MENU(MENU_ANIMA, 100, lcdseg_anima_step, __ui_show_animation);
    }
    else
    {
        UI_SHOW_MENU(MENU_ANIMA, 100, lcdseg_anima_step, __ui_show_animation);
    }
#endif
}

#if CONFIG_CLIENT_VK
void ui_show_animation(u8 task){}
static void __ui_show_change_fm_mode(void)
{
    UI_SHOW_MENU(MENU_FM_STATION, 1000, fm_manage_get_channel()&0x7F, __ui_show_animation_finish);
}
static void __ui_show_change_mode(void)
{
    extern void ms6713_change_mode_init(void);
    ms6713_change_mode_init();

    if(app_check_curr_task(APP_FM_TASK))
        UI_SHOW_MENU(MENU_CHANGE_MODE, 1000, app_get_curr_task(), __ui_show_change_fm_mode);
    else
        UI_SHOW_MENU(MENU_CHANGE_MODE, 1000, app_get_curr_task(), __ui_show_animation_finish);
}
void ui_show_change_mode(void)
{
    if(app_get_last_task() == APP_POWEROFF_TASK)
    {
        UI_SHOW_MENU(MENU_POWER_UP, 1000, 0, __ui_show_change_mode);
    }
    else if(app_get_last_task() != APP_POWERON_TASK)
    {
        __ui_show_change_mode();
    }
}
#else
void ui_show_animation(u8 task)
{
    lcdseg_anima_step = 0;
    lcdseg_anima_task = task;
    lcdseg_anima_num = rand32() % 8;

    if(lcdseg_anima_num == 0)
        memcpy(anima_table, lcdseg_anima_tbl0, sizeof(anima_table));
    else if(lcdseg_anima_num == 1)
        memcpy(anima_table, lcdseg_anima_tbl1, sizeof(anima_table));
    else if(lcdseg_anima_num == 2)
        memcpy(anima_table, lcdseg_anima_tbl2, sizeof(anima_table));
    else if(lcdseg_anima_num == 3)
        memcpy(anima_table, lcdseg_anima_tbl3, sizeof(anima_table));
    else if(lcdseg_anima_num == 4)
        memcpy(anima_table, lcdseg_anima_tbl4, sizeof(anima_table));
    else if(lcdseg_anima_num == 5)
        memcpy(anima_table, lcdseg_anima_tbl5, sizeof(anima_table));
    else if(lcdseg_anima_num == 6)
        memcpy(anima_table, lcdseg_anima_tbl6, sizeof(anima_table));
    else if(lcdseg_anima_num == 7)
        memcpy(anima_table, lcdseg_anima_tbl7, sizeof(anima_table));

    __ui_show_animation();
}
#endif

#endif
