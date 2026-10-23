#include "types.h"

void WriteLCD(u8 Byte);
void CmdLCD(u8 cmd);
void InitLCD(void);
void CharLCD(u8 asciival);
void StrLCD(s8 *str);
void U32LCD(u32 n);
void S32LCD(s32 n);
void F32LCD(f32 fn,u8 nDP);
void BuildCGRAM(u8 *p,u8 nBytes);
