//uart.c
#include <LPC21xx.h>
#include "types.h"
#include "uart_defines.h"
#include "string.h"
#include "delay.h"
#include "lcd.h"

void U0_Tx(u8);

#define BUFFER_SIZE 350
extern u32 flag,i;
extern s8 t[350];

//#if U0_TX_RX_INT_EN!=0

void U0_TxRx_ISR() __irq
{
//    u32 status;
      s8 ch;    

    if(((U0IIR>>2)&1))
    {
        ch=U0RBR;
        if(i<(BUFFER_SIZE-1))
        {
            t[i++]=ch;
            t[i]='\0';
        }    
    }
    /*if(((status>>1)&3)==THRE_INT)
    {
        flag=1;
      IOPIN0^=1<<U0_TX_STATUS_LED;
    } */
    VICVectAddr=0;
}
//#endif

void UART0_Init(void) 
{
    //cfg p0.0,p0.1 as TXD0,RXD0
    PINSEL0&=(u32)~15<<0;
    PINSEL0|=TXD0_PIN|RXD0_PIN;
    //cfg UxLCR for 8N1,DLAB enabled
    U0LCR=(1<<DLAB_BIT)|WORD_LEN;
    //cfg for speed/baudrate
    U0DLL=DIVISOR;
    U0DLM=DIVISOR>>8;
    //reset DLAB bit
    U0LCR&=~(1<<DLAB_BIT);

    U0IER|=1<<0;//THRE_INT_EN_BIT;
    
    /*#if (U0_TX_INT_EN > 0)
    {
      U0IER|=1<<0;//THRE_INT_EN_BIT;
    }    
    #endif
    
    #if (U0_RX_INT_EN > 0)
    {
      U0IER|=1<<RDA_INT_EN_BIT;
    }    
    #endif */
    
    //#if U0_TX_RX_INT_EN!=0
    //def all interrupt are irq type
    //VICIntSelect=0;
    //cfg U0 interrupt via VIC
    VICIntEnable=1<<U0_VIC_CHNO;
    //Cfg U0 interrupt as vic pri 0
    VICVectCntl0=(1<<5)|U0_VIC_CHNO;
    //load isr addr into vic slot 0
    VICVectAddr0=(u32 )U0_TxRx_ISR;
    //
    IODIR0|=(1<<U0_TX_STATUS_LED)|
            (1<<U0_RX_STATUS_LED); 
    //#endif
} 

void U0_Tx(u8 sByte) 
{ 
  U0THR=sByte;
    while(((U0LSR>>TEMT_BIT)&1)==0);
} 

u8 U0_Rx(void) 
{ 
  while(((U0LSR>>DR_BIT)&1)==0);
  return U0RBR; 
}

void U0_TxStr(s8 *p)
{
    while(*p)
        U0_Tx(*p++);
}

void U0_TxU32(u32 num)
{
    u8 a[10];
  s32 i=0;
  if(num==0)
  {
        U0_Tx('0');
  }        
    else
    {
        while(num>0)
        {
            a[i++]=(num%10)+48;
            num/=10;
        }
        for(--i;i>=0;i--)
         U0_Tx(a[i]);
    }
}

void U0_TxS32(s32 num)
{
        if(num<0)
    {
        U0_Tx('-');
        num=-num;
    }
  U0_TxU32(num);    
}

void U0_TxF32(f32 fNum,u32 nDP)
{
    u32 num;
    s32 i;
    if(fNum<0.0)
    {
        U0_Tx('-');
        fNum=-fNum;
    }
    num=fNum;
    U0_TxU32(num);
    U0_Tx('.');
    for(i=0;i<nDP;i++)
    {
        fNum=(fNum-num)*10;
        num=fNum;
        U0_Tx(num+48);
    }
}

s8*  U0_RxStr(void)
{
    static s8 rbuf[100];
    s32 i=0;
    while(1)
    {
        rbuf[i]=U0_Rx();
        U0_Tx(rbuf[i]);
        if((rbuf[i]=='\n')||(rbuf[i]=='\r'))
        {
            rbuf[i]='\0';
            break;
        }
        i++;
    }
    return rbuf;
}
