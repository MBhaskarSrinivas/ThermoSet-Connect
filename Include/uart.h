//uart.h
#include "types.h"
void UART0_Init(void);
void U0_Tx(u8 sByte);
u8   U0_Rx(void);
void U0_TxStr(s8 *);
void U0_TxU32(u32);
void U0_TxS32(s32);
void U0_TxF32(f32,u32);
s8 * U0_RxStr(void);
