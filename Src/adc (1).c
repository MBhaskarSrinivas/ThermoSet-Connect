#include "delay.h"
#include"types.h"
#include"lpc21xx.h"
#include"defines.h"
#include"adc_defines.h"

void Init_ADC(void)
{
    //cfg b/w p0.27-p0.30 as analog input pins
    PINSEL1 &= ~(u32)0xFF<<((27-16)*2);
    //PINSEL1 |= AIN0_PIN;
    PINSEL1 |= AIN1_PIN;
    ADCR |= (1<<PDN_BIT) | (CLKDIV<<CLKDIV_BITS);
}
void Read_ADC(u8 chNo,u16 *dVal,f32 *eAR)
{
    //clear prev channel selection
    ADCR &= 0xFFFFFF00;
    //select ch, start conversion 
    ADCR |= (1<<ADC_START_BIT) | (1<<chNo);
    //wait for conversion time
    delay_us(3);
    while(READBIT(ADDR,DONE_BIT)==0);
    //stop adc conversion time
    CLRBIT(ADCR,ADC_START_BIT);
    *dVal=((ADDR>>RESULT_BITS) & 1023);
    *eAR = (*dVal * (3.3/1024));
}
