#include "flash_driver.h"
#include "system/includes.h"
#include "asm/includes.h"
#include "app_config.h"


#define LOG_TAG_CONST       FLASH_IF
#define LOG_TAG             "[FLASH_IF]"
#include "debug.h"

/*
    日志标签常量，控制不同日志级别的输出
    v Verbose 详细级别 日志
    i Info   信息级别 日志
    d Debug 调试级别 日志
    w Warning 警告级别 日志
    e Error 错误级别 日志

    都被分配到 .LOG_TAG_CONST 段中
*/
const char log_tag_const_v_FLASH_IF AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_FLASH_IF AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_d_FLASH_IF AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_FLASH_IF AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_FLASH_IF AT(.LOG_TAG_CONST) = 1;

// 在 standard/isd_config.ini 中定义
///自定义flash区域空间
//  #按照芯片flash大小，最后4K保留，往上取地址
//  #USERIF_ADR=0x50000;
//  USERIF_ADR=0x5F000;
//  #USERIF_ADR=AUTO;
//  USERIF_LEN=128K;
//  USERIF_OPT=1;
// isd_config.ini 配置的地址要对齐
// 4K对齐才能SECTOR_ERASER
// 64K对齐才能BLOCK_ERASER

typedef enum _FLASH_ERASER {
    CHIP_ERASER,
    BLOCK_ERASER, // 64k
    SECTOR_ERASER,// 4k
    PAGE_ERASER,  // 256B
} FLASH_ERASER;
extern bool sfc_erase(FLASH_ERASER cmd, u32 addr);
extern u32 sdfile_cpu_addr2flash_addr(u32 offset);
extern void clr_wdt(void);

#define FLASH_AREA_NAME  SDFILE_APP_ROOT_PATH"USERIF" // 名称要与ini文件的对应
static volatile u8 rbuf[4 * 1024] __attribute__((aligned(4)));
static volatile u8 wbuf[4 * 1024] __attribute__((aligned(4)));
// FILE* zone_fp = NULL;

/**
 * @brief
 *      注意：内部有 fopen 操作
 *
 * @param path
 * @return FILE*
 */
FILE* flash_area_init(const char* path)
{
    // log_info("%s", path);
    FILE* fp = NULL;
    // struct vfs_attr attr = { 0 };
    fp = fopen(path, "r+w");
    if (fp == NULL) {
        // log_info("%s[%s]", __func__, "open fail!!!");
        return NULL;
    }
    // fget_attrs(fp, &attr);
    // log_info("%s[cpu_addr:0x%x,fsize:%dK]", __func__, attr.sclust, attr.fsize / 1024);
    // u32 flash_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
    // log_info("%s[flash_addr:0x%x,fsize:%dK]", __func__, flash_addr, attr.fsize / 1024);
    return fp;
}

int flash_area_erase(FILE* fp)
{
    // log_info("%s[0x%x]", __func__, fp);
    if (fp == NULL) {
        return -1;
    }

    struct vfs_attr attr = { 0 };
    fget_attrs(fp, &attr);
    u32 erase_total_size = attr.fsize;
    u32 erase_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
    u32 erase_size = 4096;
    u32 erase_cmd = SECTOR_ERASER;
    // log_info("%s[0x%x %dK]", __func__, erase_addr, erase_total_size / 1024);
    // log_info("%s 64K align:%s", __func__, erase_addr % 0x10000 ? "flase" : "true");
    // log_info("%s 4K align:%s", __func__, erase_addr % 0x1000 ? "flase" : "true");
    while (erase_total_size) {
        clr_wdt();
        //擦除区域操作
        sfc_erase(erase_cmd, erase_addr);
        erase_addr += erase_size;
        erase_total_size -= erase_size;
    }
    /* sfc_erase(BLOCK_ERASER, erase_addr); */
    /* sfc_erase(SECTOR_ERASER, erase_addr); */
    //擦除完成后把文件指针定位到可写的位置
    fseek(fp, 0, SEEK_SET);
    return 0;
}

/**
 * @brief 根据要写入的数据大小，按页擦除flash
 * @attention 注意：还未验证，不确定它是否能真的按页擦除
 * 
 * @param fp 
 * @param prepare_write_size 
 * @return int 
 */
int flash_area_erase_page(FILE* fp, u32 prepare_write_size)
{
    const u32 erase_cmd = PAGE_ERASER;
    const u32 erase_size_step = 256; // 每次擦除的步长：256字节
    u32 erase_addr = 0;
    struct vfs_attr attr = { 0 };

    if (fp == NULL) {
        return -1;
    }

    fget_attrs(fp, &attr);
    erase_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
    while (prepare_write_size) {
        clr_wdt();
        //擦除区域操作
        sfc_erase(erase_cmd, erase_addr);
        erase_addr += erase_size_step;
        
        if (prepare_write_size >= erase_size_step) {
            prepare_write_size -= erase_size_step;
        } 
        else {
            prepare_write_size = 0;
        }
    }

    //擦除完成后把文件指针定位到可写的位置
    fseek(fp, 0, SEEK_SET);
    return 0;
}

// 根据传入的文件指针变量，得到其指向的文件大小
u32 flash_area_size(FILE* fp)
{
    if (fp == NULL) {
        return -1;
    }
    struct vfs_attr attr = { 0 };
    fget_attrs(fp, &attr);
    log_info("%s[size:%dK]", __func__, attr.fsize / 1024);
    return attr.fsize;
}

/**
 * @brief 固定从用户配置的flash区域读出4K数据，再放入指定的缓冲区
 *
 * @param buf 存放读出的数据的缓冲区
 * @param size 读出数据的大小，需要小于等于4K，并且不能大于传参 buf 的大小
 *
 * @return 0:成功
 */
int flash_area_read_4K(u8* buf, u32 read_size)
{
    FILE* fp = NULL;
    int ret = 0;
    int start_addr = 0;
    // u32 size = 0;// 调试时使用

    if (read_size > 4 * 1024) {
        return 1;
    }

    // fp = flash_area_init(FLASH_AREA_NAME); // 内部有 fopen 操作
    fp = fopen(FLASH_AREA_NAME, "r+w");
    if (fp == NULL) {
        return 1;
    }

    // size = flash_area_size(fp);
    // printf("user flash size == %lu\n", size);

    // 清空接收缓冲区的数据
    memset(rbuf, 0x00, ARRAY_SIZE(rbuf));
    fseek(fp, 0, SEEK_SET); // 偏移到开头
    start_addr = fpos(fp); // 获取当前文件指针实际的位置
    // ret = fread(fp, rbuf, ARRAY_SIZE(rbuf));
    ret = fread(fp, rbuf, read_size);
    if (ret != read_size) {
        // printf("[%s] ret == %d\n", __func__, ret);
        return 2;
    }

    memcpy(buf, rbuf, read_size);
    fclose(fp);

    // 读取成功
    return 0;
}

int flash_area_write_4K(u8* buf, u32 write_size)
{
    FILE* fp = NULL;
    int ret = 0;
    // int start_addr = 0;
    // u32 size = 0; // 调试时使用

    if (write_size > 4 * 1024) {
        return 1;
    }

    fp = fopen(FLASH_AREA_NAME, "r+w");
    if (fp == NULL) {
        return 2;
    }

    ret = flash_area_erase(fp); // 擦除整个区域
    fseek(fp, 0, SEEK_SET);
    memcpy(wbuf, buf, write_size);
    ret = fwrite(fp, wbuf, write_size);
    if (ret != write_size) {
        return 3;
    }

    fclose(fp);
    return 0;
}

#if 0 // 测试程序：

/**
 * @brief 测试 4K 区域读写，不能放在执行时间较短的线程和定时中断中
 *
 */
void user_flash_area_test(void)
{
    static volatile u8 buff[4 * 1024] = { 0 };
    u16 buff_index = 0;
    for (u8 i = 0; i < 32; i++) // 重复32次，32 * 128 == 4 * 1024
    {
        for (u8 j = 0; j < 128; j++)
        {
            buff[buff_index] = j;
            buff_index++;
        }
    }

    if (buff_index == 4 * 1024)
    {
        printf("buff init success\n");
    }
    else
    {
        printf("buff init fail\n");
    }

    flash_area_write_4K((u8*)buff, ARRAY_SIZE(buff));
    memset(buff, 0, ARRAY_SIZE(buff));
    flash_area_read_4K((u8*)buff, ARRAY_SIZE(buff));
    printf("\n=============================================\n");
    // printf_buf(buff, ARRAY_SIZE(buff));
    for (u16 i = 0; i < ARRAY_SIZE(buff); i++)
    {
        printf("buff [%u] == %u ", i, buff[i]);
    }
    printf("\n============================================^\n");
}


///使用范例
#define FLASH_AREA_NAME  SDFILE_APP_ROOT_PATH"USERIF" //名称要与ini文件对应
static u8 rbuf[4 * 1024] __attribute__((aligned(4)));
static u8 wbuf[4 * 1024] __attribute__((aligned(4)));
FILE* zone_fp = NULL;
void flash_area_test(void)
{
    log_info("%s", __func__);

    zone_fp = flash_area_init(FLASH_AREA_NAME);
    if (zone_fp == NULL) {
        return;
    }
    int ret = 0;
    FILE* fp = zone_fp;
    int start_addr = 0;
    u32 size = flash_area_size(fp);
    // 清空接收缓冲区的数据
    memset(rbuf, 0x00, sizeof(rbuf));
    log_info("%s[memset rbuf 0x00]", __func__);
    put_buf(rbuf, 16);
    // 偏移到开头
    fseek(fp, 0, SEEK_SET);
    start_addr = fpos(fp); // 获取当前文件指针实际的位置
    ret = fread(fp, rbuf, sizeof(rbuf)); //读第1个扇区4K数据
    log_info("%s[fread:0x%x %d]", __func__, start_addr, ret);
    put_buf(rbuf, 16);

    fseek(fp, size - 4096, SEEK_SET);
    start_addr = fpos(fp);
    ret = fread(fp, rbuf, sizeof(rbuf));//读最后的扇区4K数据
    log_info("%s[fread:0x%x %d]", __func__, start_addr, ret);
    put_buf(rbuf, 16);

    ret = flash_area_erase(fp); //擦除整个区域
    log_info("%s[reset:%d]", __func__, ret);

    for (u16 i = 0;i < sizeof(wbuf); i++) {
        wbuf[i] = rbuf[i] + i;
    }

    fseek(fp, 0, SEEK_SET);
    start_addr = fpos(fp);
    ret = fwrite(fp, wbuf, sizeof(wbuf)); //写第1个扇区4K数据
    log_info("%s[fwrite:0x%x %d]", __func__, start_addr, ret);
    put_buf(wbuf, 16);

    fseek(fp, size - 4096, SEEK_SET);
    start_addr = fpos(fp);
    ret = fwrite(fp, wbuf, sizeof(wbuf)); //写最后的扇区4K数据
    log_info("%s[fwrite:0x%x %d]", __func__, start_addr, ret);
    put_buf(wbuf, 16);

    memset(rbuf, 0x00, sizeof(rbuf));
    log_info("%s[memset rbuf 0x00]", __func__);
    put_buf(rbuf, 16);

    fseek(fp, 0, SEEK_SET);
    start_addr = fpos(fp);
    ret = fread(fp, rbuf, sizeof(rbuf)); //读第1个扇区4K数据
    log_info("%s[fread:0x%x %d]", __func__, start_addr, ret);
    put_buf(rbuf, 16);

    fseek(fp, size - 4096, SEEK_SET);
    start_addr = fpos(fp);
    ret = fread(fp, rbuf, sizeof(rbuf));//读最后的扇区4K数据
    log_info("%s[fread:0x%x %d]", __func__, start_addr, ret);
    put_buf(rbuf, 16);

    if (0 == memcmp(rbuf, wbuf, sizeof(rbuf))) {
        log_info("rbuf and wbuf memcpy pass !!!!!!");
    }
    else {
        log_error("rbuf and wbuf memcpy error !!!!!!!");
    }
}

#endif

