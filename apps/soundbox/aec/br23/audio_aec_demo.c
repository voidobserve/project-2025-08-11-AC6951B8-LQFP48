/*
 ***************************************************************************
 *							AUDIO AEC DEMO
 * File  : audio_aec_demo.c
 * By    : GZR
 * Notes : 1.可用内存
 *		   (1)free_ram:	动态内存(mem_stats:physics memory size xxxx bytes)
 *		   (2)mux_rx_bulk:蓝牙复用空间(MUX_RX_BULK_MAX)
 *		   2.demo默认将输入数据copy到输出，相关处理只需在运算函数
 *		    audio_aec_run()实现即可
 *		   3.双mic ENC开发，只需打开对应板级以下配置即可：
 *			#define TCFG_AUDIO_DUAL_MIC_ENABLE		ENABLE_THIS_MOUDLE
 *		   4.建议算法开发者使用宏定义将自己的代码模块包起来
 *		   5.算法处理完的数据，如有需要，可以增加EQ处理：AEC_UL_EQ_EN
 *		   6.开发阶段，默认使用芯片最高主频240MHz，可以通过修改AEC_CLK来修改
 			运行频率。
 ***************************************************************************
 */
#include "aec_user.h"
#include "system/includes.h"
#include "media/includes.h"
#include "application/audio_eq_drc_apply.h"
#include "circular_buf.h"
#include "debug.h"

#if defined(TCFG_CVP_DEVELOP_ENABLE) && (TCFG_CVP_DEVELOP_ENABLE == 1)

#define AEC_MALLOC_ENABLE	0   /*是否使用动态内存*/

#define AEC_EXTERN_REFERENCE_ENABLE   0 /*是否使用外部取的参考dac数据*/

#define AEC_CLK				(160 * 1000000L)	/*模块运行时钟(MaxFre:240MHz)*/
#define AEC_FRAME_POINTS	256					/*AEC处理帧长，跟mic采样长度关联*/
#define AEC_FRAME_SIZE		(AEC_FRAME_POINTS << 1)

#if TCFG_AUDIO_DUAL_MIC_ENABLE  /*ac695不支持双mic*/
#define AEC_MIC_NUM			2	/*双mic*/
#else
#define AEC_MIC_NUM			1	/*单mic*/
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/

#ifdef AUDIO_PCM_DEBUG
/*AEC串口数据导出*/
const u8 CONST_AEC_EXPORT = 1;
#else
const u8 CONST_AEC_EXPORT = 0;
#endif/*AUDIO_PCM_DEBUG*/

/*上行数据eq*/
#define AEC_UL_EQ_EN		0//TCFG_AEC_UL_EQ_ENABLE


extern int audio_dac_read_reset(void);
extern int audio_dac_read(s16 points_offset, void *data, int len);
extern void esco_enc_resume(void);
extern int aec_uart_init();
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);

/*
 *蓝牙收数buf复用使用说明
 *(1)打开复用：apps/earphone/log_config/lib_btctrler_config.c
 *	const int CONFIG_ESCO_MUX_RX_BULK_ENABLE  =  1;
 *(2)MUX_RX_BULK_TEST_DEMO是使用范例
 *(3)zalloc_mux_rx_bulk/free_mux_rx_bulk用法和普通malloc/free一样
 */
//#define MUX_RX_BULK_TEST_DEMO						/*蓝牙收数buf复用使用范例*/
#define MUX_RX_BULK_MAX			(9 * 1024 - 100)	/*蓝牙收数buf最多可以申请到7k*/
extern void *zalloc_mux_rx_bulk(int size);
extern void free_mux_rx_bulk(void *buf);
int *mux_rx_bulk_test = NULL;

/*AEC输入buf复用mic_adc采样buf*/
#define MIC_BULK_MAX		3
struct mic_bulk {
    struct list_head entry;
    s16 *addr;
    u16 len;
    u16 used;
};

struct audio_aec_hdl {
    u8 start;
    volatile u8 busy;
    u8 output_buf[1000];	/*aec模块输出缓存*/
    cbuffer_t output_cbuf;
    s16 *mic;				/*主mic数据地址*/
    s16 *mic_ref;			/*参考mic数据地址*/
    s16 *free_ram;			/*当前可用内存*/
    s16 spk_ref[AEC_FRAME_POINTS];	/*扬声器参考数据*/
    s16 out[AEC_FRAME_POINTS];		/*运算输出地址*/
    OS_SEM sem;
    /*数据复用相关数据结构*/
    struct mic_bulk in_bulk[MIC_BULK_MAX];
    struct mic_bulk inref_bulk[MIC_BULK_MAX];
    struct list_head in_head;
    struct list_head inref_head;
#if AEC_UL_EQ_EN
    struct audio_eq *ul_eq;
#endif/*AEC_UL_EQ_EN*/
    struct aec_s_attr attr;
#if AEC_EXTERN_REFERENCE_ENABLE
    u8 extern_ref_buf[1000];	/*aec外部参考数据缓存*/
    cbuffer_t extern_ref_cbuf;
#endif // AEC_EXTERN_REFERENCE_ENABLE
};
struct audio_aec_hdl *aec_hdl = NULL;
#if (AEC_MALLOC_ENABLE == 0)
struct audio_aec_hdl  aec_hdl_mem AT(.aec_mem);
#endif /*AEC_MALLOC_ENABLE*/

extern void esco_adc_mic_en();
void audio_aec_ref_start(u8 en)
{
    if (aec_hdl) {
        if (en != aec_hdl->attr.fm_tx_start) {
            aec_hdl->attr.fm_tx_start = en;
            esco_adc_mic_en();
            y_printf("fm_tx_start:%d\n", en);
        }
    }
}

int audio_aec_output_read(s16 *buf, u16 len)
{
    local_irq_disable();
    if (!aec_hdl || !aec_hdl->start) {
        printf("audio_aec close now");
        local_irq_enable();
        return -EINVAL;
    }
    u16 rlen = cbuf_read(&aec_hdl->output_cbuf, buf, len);
    if (rlen == 0) {
        //putchar('N');
    }
    local_irq_enable();
    return rlen;
}

static int audio_aec_output(s16 *buf, u16 len)
{
    u16 wlen = 0;
    if (aec_hdl && aec_hdl->start) {
        wlen = cbuf_write(&aec_hdl->output_cbuf, buf, len);
        if (wlen != len) {
            printf("aec_out_full:%d,%d\n", len, wlen);
        }
        esco_enc_resume();
    }
    return wlen;
}

#if AEC_UL_EQ_EN
static int ul_eq_output(void *priv, void *data, u32 len)
{
    return 0;
}
#endif/*AEC_UL_EQ_EN*/

/*
 *跟踪系统内存使用情况:physics memory size xxxx bytes
 *正常的系统运行过程，应该至少有3k bytes的剩余空间给到系统调度开销
 */
static void sys_memory_trace(void)
{
    static int cnt = 0;
    if (cnt++ > 200) {
        cnt = 0;
        mem_stats();
    }
}

/*
*********************************************************************
*                  Audio AEC RUN
* Description: AEC数据处理核心
* Arguments  : in 		主mic数据
*			   inref	参考mic数据(双mic降噪有用)
*			   ref		speaker参考数据
*			   out		数据输出
* Return	 : 数据运算输出长度
* Note(s)    : 在这里实现AEC_core
*********************************************************************
*/
static int audio_aec_run(s16 *in, s16 *inref, s16 *ref, s16 *out, u16 points)
{
    int out_size = 0;
    putchar('.');
    memcpy(out, in, (points << 1));
    //memcpy(out, inref, (points << 1));
    out_size = points << 1;
#if AEC_UL_EQ_EN
    if (aec_hdl->ul_eq) {
        //putchar('Q');
        audio_eq_run(aec_hdl->ul_eq, out, out_size);
    }
#endif/*AEC_UL_EQ_EN*/
    sys_memory_trace();
    return out_size;
}

/*
*********************************************************************
*                  Audio AEC Task
* Description: AEC任务
* Arguments  : priv	私用参数
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void audio_aec_task(void *priv)
{
    printf("==Audio AEC Task==\n");
    struct mic_bulk *bulk = NULL;
    struct mic_bulk *bulk_ref = NULL;
    u8 pend = 1;
    while (1) {
        if (pend) {
            os_sem_pend(&aec_hdl->sem, 0);
        }
        pend = 1;
        if (aec_hdl->start) {
            if (!list_empty(&aec_hdl->in_head)) {
                aec_hdl->busy = 1;
                local_irq_disable();
                /*1.获取主mic数据*/
                bulk = list_first_entry(&aec_hdl->in_head, struct mic_bulk, entry);
                list_del(&bulk->entry);
                aec_hdl->mic = bulk->addr;
#if (AEC_MIC_NUM == 2)
                /*2.获取参考mic数据*/
                bulk_ref = list_first_entry(&aec_hdl->inref_head, struct mic_bulk, entry);
                list_del(&bulk_ref->entry);
                aec_hdl->mic_ref = bulk_ref->addr;
#endif/*Dual_Microphone*/
                local_irq_enable();
#if AEC_EXTERN_REFERENCE_ENABLE
                /*3.获取外部参考数据*/
                //audio_dac_read(60, aec_hdl->spk_ref, AEC_FRAME_SIZE);
                cbuf_read(&aec_hdl->extern_ref_cbuf, aec_hdl->spk_ref, AEC_FRAME_SIZE);
#else
                /*3.获取speaker参考数据*/
                audio_dac_read(60, aec_hdl->spk_ref, AEC_FRAME_SIZE);
#endif //AEC_EXTERN_REFERENCE_ENABLE
                /*4.算法处理*/
                int out_len = audio_aec_run(aec_hdl->mic, aec_hdl->mic_ref, aec_hdl->spk_ref, aec_hdl->out, AEC_FRAME_POINTS);

                /*5.结果输出*/
                audio_aec_output(aec_hdl->out, out_len);

                /*6.数据导出*/
                if (CONST_AEC_EXPORT) {
                    aec_uart_fill(0, aec_hdl->mic, 512);		//主mic数据
                    //  aec_uart_fill(1, aec_hdl->mic_ref, 512);	//副mic数据
                    aec_uart_fill(1, aec_hdl->spk_ref, 512);	//扬声器数据
                    aec_uart_fill(2, aec_hdl->out, out_len);	//算法运算结果
                    aec_uart_write();

                }
                bulk->used = 0;
#if (AEC_MIC_NUM == 2)
                bulk_ref->used = 0;
#endif/*Dual_Microphone*/
                aec_hdl->busy = 0;
                pend = 0;
            }
        }
    }
}

/*
*********************************************************************
*                  Audio AEC Init
* Description: 初始化AEC模块
* Arguments  : sample_rate 采样率(8000/16000)
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_aec_init(u16 sample_rate)
{
    printf("audio_aec_init,sr = %d\n", sample_rate);
    mem_stats();
    if (aec_hdl) {
        return -1;
    }
#if AEC_MALLOC_ENABLE
    aec_hdl = zalloc(sizeof(struct audio_aec_hdl));
#else /*AEC_MALLOC_ENABLE == 0*/
    memset(&aec_hdl_mem, 0, sizeof(aec_hdl_mem));
    aec_hdl = &aec_hdl_mem;
#endif /*AEC_MALLOC_ENABLE*/
    printf("aec_hdl size:%d\n", sizeof(struct audio_aec_hdl));
    clk_set("sys", AEC_CLK);
    INIT_LIST_HEAD(&aec_hdl->in_head);
    INIT_LIST_HEAD(&aec_hdl->inref_head);
    cbuf_init(&aec_hdl->output_cbuf, aec_hdl->output_buf, sizeof(aec_hdl->output_buf));
#if AEC_EXTERN_REFERENCE_ENABLE
    cbuf_init(&aec_hdl->extern_ref_cbuf, aec_hdl->extern_ref_buf, sizeof(aec_hdl->extern_ref_buf));
#endif // AEC_EXTERN_REFERENCE_ENABLE
    if (CONST_AEC_EXPORT) {
        aec_uart_init();
        printf("aec_uart_init \n");
    }
#if AEC_UL_EQ_EN
    struct audio_eq_param ul_eq_param = {0};
    ul_eq_param.sr = sample_rate;
    ul_eq_param.channels = 1;
    ul_eq_param.online_en = 1;
    ul_eq_param.mode_en = 0;
    ul_eq_param.remain_en = 0;
    ul_eq_param.max_nsection = EQ_SECTION_MAX;
    ul_eq_param.cb = aec_ul_eq_filter;
    ul_eq_param.eq_name = aec_eq_mode;
    aec_hdl->ul_eq = audio_dec_eq_open(&ul_eq_param);
    /* audio_eq_open(&aec_hdl->ul_eq, &ul_eq_param); */
    /* audio_eq_set_samplerate(&aec_hdl->ul_eq, sample_rate); */
    /* audio_eq_set_output_handle(&aec_hdl->ul_eq, ul_eq_output, NULL); */
    /* audio_eq_start(&aec_hdl->ul_eq); */
#endif/*AEC_UL_EQ_EN*/
    os_sem_create(&aec_hdl->sem, 0);
    task_create(audio_aec_task, NULL, "aec");
    audio_dac_read_reset();
    aec_hdl->start = 1;

    mem_stats();
#if	0
    aec_hdl->free_ram = malloc(1024 * 63);
    mem_stats();
#endif

#ifdef MUX_RX_BULK_TEST_DEMO
    mux_rx_bulk_test = zalloc_mux_rx_bulk(MUX_RX_BULK_MAX);
    if (mux_rx_bulk_test) {
        printf("mux_rx_bulk_test:0x%x\n", mux_rx_bulk_test);
        free_mux_rx_bulk(mux_rx_bulk_test);
        mux_rx_bulk_test = NULL;
    }
#endif/*MUX_RX_BULK_TEST_DEMO*/

    printf("audio_aec_init succ\n");
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Close
* Description: 关闭AEC模块
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_aec_close(void)
{
    printf("audio_aec_close succ\n");
    if (aec_hdl) {
        aec_hdl->start = 0;
        while (aec_hdl->busy) {
            os_time_dly(2);
        }
        task_kill("aec");
        if (CONST_AEC_EXPORT) {
            aec_uart_close();
        }
#if AEC_UL_EQ_EN
        if (aec_hdl->ul_eq) {
            audio_dec_eq_close(aec_hdl->ul_eq);
        }
#endif/*AEC_UL_EQ_EN*/
        local_irq_disable();
        if (aec_hdl->free_ram) {
            free(aec_hdl->free_ram);
        }
#if AEC_MALLOC_ENABLE
        free(aec_hdl);
#endif /*AEC_MALLOC_ENABLE*/
        aec_hdl = NULL;
        local_irq_enable();
        printf("audio_aec_close succ\n");
    }
}

/*
*********************************************************************
*                  Audio AEC Input
* Description: AEC源数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度
* Return	 : None.
* Note(s)    : 输入一帧数据，唤醒一次运行任务处理数据，默认帧长256点
*********************************************************************
*/
void audio_aec_inbuf(s16 *buf, u16 len)
{
    if (aec_hdl && aec_hdl->start) {
        int i = 0;
        for (i = 0; i < MIC_BULK_MAX; i++) {
            if (aec_hdl->in_bulk[i].used == 0) {
                break;
            }
        }
        if (i < MIC_BULK_MAX) {
            aec_hdl->in_bulk[i].addr = buf;
            aec_hdl->in_bulk[i].used = 0x55;
            aec_hdl->in_bulk[i].len = len;
            list_add_tail(&aec_hdl->in_bulk[i].entry, &aec_hdl->in_head);
        } else {
            printf(">>>aec_in_full\n");
            /*align reset*/
            struct mic_bulk *bulk;
            list_for_each_entry(bulk, &aec_hdl->in_head, entry) {
                bulk->used = 0;
                __list_del_entry(&bulk->entry);
            }
            return;
        }
        os_sem_set(&aec_hdl->sem, 0);
        os_sem_post(&aec_hdl->sem);
    }
}

void audio_aec_inbuf_ref(s16 *buf, u16 len)
{
    if (aec_hdl && aec_hdl->start) {
        int i = 0;
        for (i = 0; i < MIC_BULK_MAX; i++) {
            if (aec_hdl->inref_bulk[i].used == 0) {
                break;
            }
        }
        if (i < MIC_BULK_MAX) {
            aec_hdl->inref_bulk[i].addr = buf;
            aec_hdl->inref_bulk[i].used = 0x55;
            aec_hdl->inref_bulk[i].len = len;
            list_add_tail(&aec_hdl->inref_bulk[i].entry, &aec_hdl->inref_head);
        } else {
            printf(">>>aec_inref_full\n");
            /*align reset*/
            struct mic_bulk *bulk;
            list_for_each_entry(bulk, &aec_hdl->inref_head, entry) {
                bulk->used = 0;
                __list_del_entry(&bulk->entry);
            }
            return;
        }
    }
}

/*
*********************************************************************
*                  Audio AEC Reference
* Description: AEC模块参考数据输入
* Arguments  : buf	输入参考数据地址
*			   len	输入参考数据长度
* Return	 : None.
* Note(s)    : 声卡设备是DAC，默认不用外部提供参考数据
*********************************************************************
*/
void audio_aec_refbuf(s16 *buf, u16 len)
{
#if AEC_EXTERN_REFERENCE_ENABLE
    u16 wlen = 0;
    if (aec_hdl && aec_hdl->start) {
        wlen = cbuf_write(&aec_hdl->extern_ref_cbuf, buf, len);
    }
#endif // AEC_EXTERN_REFERENCE_ENABLE
}





void aec_cfg_fill(AEC_CONFIG *cfg)
{
}
void audio_aec_dlyest_en(void)
{

}

#endif /*TCFG_CVP_DEVELOP_ENABLE*/

