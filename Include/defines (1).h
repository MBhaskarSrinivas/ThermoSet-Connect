//defines.h
#define WRITEBYTE(WORD,STARTBIT,BYTE) \
        WORD=(WORD&(u32)~0xFF<<STARTBIT)| \
				     (BYTE<<STARTBIT)
						 
#define READBIT(WORD,BIT) ((WORD>>BIT)&1)						 

#define WRITEBIT(WORD,BIT,BITLEVEL) \
        WORD=((WORD&~(1<<BIT))|(BITLEVEL<<BIT))
				
#define READWRITEBIT(WORD,DBIT,SBIT) \
        WORD=((WORD&~(1<<DBIT))| \
				      (((WORD>>SBIT)&1)<<DBIT))

#define READWRITEBIT2(DWORD,DBIT,SWORD,SBIT) \
        DWORD=((DWORD&~(1<<DBIT))| \
				      (((SWORD>>SBIT)&1)<<DBIT))
							
#define WRITENIBBLE(WORD,STARTBIT,NIBBLE) \
        WORD=(WORD&(u32)~0xF<<STARTBIT)| \
				     (NIBBLE<<STARTBIT)
						 
#define READNIBBLE(WORD,SBIT) ((WORD>>SBIT)&15)

#define CLRBIT(WORD,BIT) WORD&=~(1<<BIT)
//add more defines as & when required
