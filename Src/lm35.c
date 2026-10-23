//lm35.c

#include "types.h"
#include "adc.h"

u32 Read_Temperature(void)
{
		u16 dVal;  f32 eAR; 
    	static u32 flag	;
	  	if(flag==0)
		{	
		  	Init_ADC();
			flag=1;
		}	
		Read_ADC(1,&dVal,&eAR);
		return eAR*100;
}

s32 Read_TemperatureNP(void)
{
	u16 dVal1,dVal2;  
	f32 eAR1,eAR2; 
   	static u32 flag	;
	if(flag==0)
	{	
	  	Init_ADC();
		flag=1;
	}	
	Read_ADC(0,&dVal1,&eAR1);
	Read_ADC(1,&dVal2,&eAR2);
	return ((eAR1-eAR2)*100);
}
