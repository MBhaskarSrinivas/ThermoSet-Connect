#include "types.h"

void Clear_Buffer(void);
u32 waitforresponse(s8 *target_keyword,u32 timeout_ms);
u8 thinkspeak_write(s8 *write_key,u32 temp,u8 alert);
s32 thinkspeak_read(s8 *channel_id, s8 *read_key);
