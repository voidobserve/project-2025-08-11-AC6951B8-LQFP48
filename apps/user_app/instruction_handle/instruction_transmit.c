// encoding : utf-8
// instruction_transmit.c 包含给下位机传递指令的接口 
// 包含接收来自下位机信息的接口
#include "instruction_transmit.h"

#include "user_uart1_driver.h"
#include "user_uart2_driver.h"

// 给下位机传递信息
void instruction_transmit_buff(u8* buff, u8 len)
{
    Uart1_Send_Tx(buff, len);
}

