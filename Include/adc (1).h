//adc.h
#include "types.h"

void Init_ADC(void);
void Read_ADC(u8 chNo,u16 *dVal,f32 *eAR);
