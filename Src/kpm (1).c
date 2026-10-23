#include<lpc21xx.h>
#include"types.h"
#include"defines.h"
#include"kpm_defines.h"
#include"lcd_defines.h"
#include"lcd.h"
#include"delay.h"

/*u32 kpmLUT[4][4]=
{
    {1,2,3,4},{5,6,7,8},
    {9,10,11,12},{13,14,15,16}
};*/
 //for key lcd display
u32 kpmLUT[4][4]=
{
    {'7','8','9','S'},{'4','5','6','P'},
    {'1','2','3','N'},{'C','0','=','B'}
};

void Init_KPM(void)
{
    //cfg rows(p1.16-p1.19) as outputs
    WRITENIBBLE(IODIR1,ROW0,15);
}
u32 colscan(void)
{
    if(READNIBBLE(IOPIN1,COL0)<15)
        return 0;
    else
        return 1;
}
u32 rowcheck(void)
{
    u32 rno;
    for(rno=0;rno<4;rno++)
    {
        WRITENIBBLE(IOPIN1,ROW0,((~(1<<rno))&0x0F));
        if(colscan()==0)
            break;
    }
    //make rows as defaulf
    IOCLR1 = 15<<ROW0;
    return rno;
}
u32 colcheck(void)
{
    u32 cno;
    for(cno=0;cno<4;cno++)
    {
        if(READBIT(IOPIN1,COL0+cno)==0)
            break;
    }
    return cno;
}
u32 keyscan(void)
{
    u32 rno,cno,key;
    //wait for switch press
    while(colscan());

    //find rno;
    rno=rowcheck();
    //find cno
    cno=colcheck();
    //collect key using kmpLUT
    //while(!colscan());
    key=kpmLUT[rno][cno];
    //wait for key release
    delay_ms(100);
     while(!colscan());
    return key;
}
u32 ReadNUM(void)
{
        u8 key;
        u32 num=0,digit=0,flag=0;
        while(1)
        {
        key=keyscan();
        if(key<= '9' && key>='0')
        {
        if(digit<4)
        {
        flag=1;
        CharLCD(key);
        num=num*10 + (key-48);
        digit++;
        }
        }
        else if(key=='B')
        {
        if(digit>0)
        {
        digit--;
        num/=10;
        }
        CmdLCD(GOTO_LINE2_POS0);
        CmdLCD(GOTO_LINE2_POS0+digit);
        CharLCD(' ');
        CmdLCD(GOTO_LINE2_POS0);
        if(digit!=0)U32LCD(num);
        if(digit==0)flag=0;
        }
        else if(key=='C')
        return 0xFFFFFFFF;
        else if(key=='E' || key=='S' )
            {
                    if(flag==1) return num;
                    else        return 0xFFFFFFFE;
            }
        else if(key=='N')
        return 0xFFFFFFFD;
        else if(key=='P')
        return 0xFFFFFFFC;
        else
        break;
        }
    return num;
}
