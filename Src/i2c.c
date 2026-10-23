//i2c.c
#include <LPC21xx.h>
#include "types.h"
#include "i2c_defines.h"
#include "defines.h"

void init_i2c(void)
{
	 //Configure I/O pin for SCL & SDA 
	 //functions 
	 PINSEL0&=(u32)~(15<<3);
	 PINSEL0|=SCL_PIN_FUNC|SDA_PIN_FUNC;
	 //Configure Speed for I2C Transfer
	 I2SCLL=LOADVAL;
	 I2SCLH=LOADVAL;
	//I2C Peripheral Enable for Communication
	 I2CONSET=1<<I2EN_BIT; 
}	

void i2c_start(void)
{
	// issue start condition
  I2CONSET=1<<STA_BIT;
	//wait for start bit status
	while(((I2CONSET>>SI_BIT)&1)==0);
	// clear start condition
	I2CONCLR=1<<STAC_BIT;
}	

void i2c_restart(void)
{
	// re-start condition
	I2CONSET=1<<STA_BIT;
	//clr SI_BIT
	I2CONCLR=1<<SIC_BIT;
	//wait for SI bit status
	while(((I2CONSET>>SI_BIT)&1)==0);
	// clear re-start condition
	I2CONCLR=1<<STAC_BIT;
}	

void i2c_write(u8 dat)
{
	//put data into I2DAT
	I2DAT=dat;	
	//clr SI_BIT
	I2CONCLR = 1<<SIC_BIT;
	//wait for SI bit status
	while(((I2CONSET>>SI_BIT)&1)==0);
}	

void i2c_stop(void)
{
	// issue stop condition
	I2CONSET=1<<STO_BIT;
  // clr SIC bit status	
	I2CONCLR = 1<<SIC_BIT;
	//stop will cleared automatically
	while(((I2CONSET>>STO_BIT)&1));
	
}

u8 i2c_nack(void)
{	
	//Assert Not of Ack
	I2CONCLR = 1<<AAC_BIT;
  //initiate serialization in & issue nack	
  I2CONCLR = 1<<SIC_BIT;
	while(((I2CONSET>>SI_BIT)&1)==0);
	return I2DAT;
}

u8 i2c_mack(void)
{	
	//Assert Ack
	I2CONSET = 1<<AA_BIT; 
	//initiate serialization in & issue nack	
  I2CONCLR = 1<<SI_BIT;
	while(((I2CONSET>>SI_BIT)&1)==0);
	//Clear Assert Ack
	I2CONCLR = 1<<AAC_BIT; 
 	return I2DAT;
}	
