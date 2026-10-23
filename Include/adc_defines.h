//adc_defines
#include "types.h"
#define FOSC 12000000 //on HW Kit
#define CCLK (FOSC*5) //as per startup.s
#define PCLK (CCLK/4) //as per startup.s
#define ADCCLK 3000000

//defines for ADCR bits
#define CLKDIV_BITS    8
#define PDN_BIT        21
#define ADC_START_BIT  24

//defines for ADDR bits
#define RESULT_BITS 6
#define CHN_BITS    24
#define DONE_BIT    31

//defines for Pin Connect Block
#define AIN0_PIN 0x00400000
#define AIN1_PIN 0x01000000
#define AIN2_PIN 0
#define AIN3_PIN 0

#define CLKDIV ((PCLK/ADCCLK)-1)
