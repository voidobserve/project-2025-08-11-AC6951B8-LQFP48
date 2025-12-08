#ifndef __USER_UART1_DRIVER_H__
#define __USER_UART1_DRIVER_H__

#include "includes.h"
#include "../../../apps/user_app/user_config.h"

u8 Uart1_Init(void);
void Uart1_Send_Tx(u8* txBuf, u8 len);
// void uart1_tx_byte(u8 byte);

#endif
