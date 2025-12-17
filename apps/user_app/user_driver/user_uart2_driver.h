#ifndef __USER_UART2_DRIVER_H__
#define __USER_UART2_DRIVER_H__ 

#include "includes.h"
u8 Uart2_Init(void);
void Uart2_Send_Tx(u8* txBuf, u8 len);

#endif