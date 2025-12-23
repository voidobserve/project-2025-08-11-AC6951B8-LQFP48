#include "instruction_feedback.h"
#include "user_uart1_driver.h"
#include "user_uart2_driver.h"


/**
 * @brief 给上一级设备/PC反馈信息
 *      可以用 sprintf() 先准备好数据，再调用该函数
 *
 *      len = sprintf();
 *      instruction_feedback_buffer(buffer, len);
 *
 * @param buffer
 * @param len
 */
void instruction_feedback_buffer(u8* buffer, u8 len)
{
    // extern void Uart2_Send_Tx(u8* txBuf, u8 len);
    // Uart2_Send_Tx("set time ok \n", sizeof("set time ok \n"));
    Uart2_Send_Tx(buffer, len);
}


// /**
//  * @brief 给上一级设备/PC反馈信息（固定长度的字符串）
//  * 
//  * @param str 
//  * @return * void 
//  */
// void instruction_feedback_str(u8 *str)
// {
//     u8 buffer[50] = {0};
//     int len = sprintf(buffer, str);
//     instruction_feedback_buffer(buffer, len);    
// }

// 给上位机反馈指令执行成功的信息
void instruction_feedback_success(u8 addr, u8 cmd_type)
{
    u8 buffer[50] = { 0 };
    int len = 0;
    len = sprintf(buffer, "%x %x OK", (u16)addr, (u16)cmd_type);
    instruction_feedback_buffer(buffer, len);
}

// 给上位机反馈指令执行失败的信息
void instruction_feedback_fail(u8 addr, u8 cmd_type)
{
    u8 buffer[50] = { 0 };
    int len = 0;
    len = sprintf(buffer, "%x %x FAIL", (u16)addr, (u16)cmd_type);
    instruction_feedback_buffer(buffer, len);
}

