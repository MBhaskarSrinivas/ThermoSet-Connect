 void delay_us(unsigned int tdly)

 {

 	tdly*=12;

	while(tdly--);

 }

 void delay_ms(unsigned int tdly)

 {

 	tdly*=12000;

	while(tdly--);

 }

 void delay_s(unsigned int tdly)

 {

 	tdly*=12000000;

	while(tdly--);

 }
