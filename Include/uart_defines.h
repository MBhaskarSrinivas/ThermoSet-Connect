//softuart_defines.h
#define TXD0_PIN 0x00000001 
#define RXD0_PIN 0x00000004 

#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)
#define BAUD 9600
#define DIVISOR (PCLK/(16*BAUD))

//defines for UxLCR
#define _8BIT 3
#define WORD_LEN _8BIT
#define DLAB_BIT 7

//defines for UxLSR
#define DR_BIT   0
#define TEMT_BIT 6

#define U0_TX_INT_EN 0
#define U0_RX_INT_EN 0
#define U0_TX_RX_INT_EN 0

//define for UxIER sfr
#define RDA_INT_EN_BIT  0
#define THRE_INT_EN_BIT 1

//defines for UxIIR sfr
#define THRE_INT 1
#define RDA_INT  2

//defines for status of interrupt
#define U0_TX_STATUS_LED 8//p0.8
#define U0_RX_STATUS_LED 9//p0.9

#define U0_VIC_CHNO 6

