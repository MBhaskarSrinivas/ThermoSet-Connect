#include <LPC21xx.h>
#include "types.h"
#include "lcd_defines.h"
#include "string.h"
#include "delay.h"
#include "lcd.h"
#include "uart.h"
#include "esp01_defines.h"
#include "esp01.h"

s8 t[BUFFER_SIZE];
u32 i;

//Simple unsigned-to-decimal-string helper (no sprintf dependency)
static void u32_to_str(u32 n, s8 *out)
{
    s8 tmp[12];
    u32 k = 0, j;
    if (n == 0) { out[0]='0'; out[1]='\0'; return; }
    while (n > 0) { tmp[k++] = (n % 10) + '0'; n /= 10; }
    for (j = 0; j < k; j++) out[j] = tmp[k-1-j];
    out[k] = '\0';
}

//Line 1 = the command being sent; waitforresponse() fills line 2
//with the response/keyword once it arrives.
static void ShowCmd(s8 *label)
{
    CmdLCD(CLEAR_LCD);
    delay_ms(2);
    CmdLCD(GOTO_LINE1_POS0);
    StrLCD(label);
}


static void CloseTCPConnection(void)
{
    Clear_Buffer();
    ShowCmd("AT+CIPCLOSE");
    U0_TxStr("AT+CIPCLOSE\r\n");
    /* Do not depend on CLOSED being captured: the existing RX buffer
       mechanism remains untouched, and a later CIPSTART can proceed
       after the short guard delay. */
    delay_ms(1000);
}

void Clear_Buffer(void)
{
    memset((void*)t, '\0', BUFFER_SIZE);
    i = 0;
}

u32 waitforresponse(s8 *target_keyword, u32 timeout_ms)
{
    delay_ms(timeout_ms);
    CmdLCD(GOTO_LINE2_POS0);
    if (strstr((const char*)t, (const char*)target_keyword) != NULL)
    {
        StrLCD(target_keyword);
		StrLCD("              ");
        delay_ms(500);
        return 1;
    }
    if ((strstr((const char*)t, "ERROR") != NULL) ||
        (strstr((const char*)t, "FAIL") != NULL))
    {
        StrLCD("ERROR           ");
        delay_ms(500);
        return 0;
    }
    return 0;
}

u8 thinkspeak_write(s8 *write_key, u32 temp, u8 alert)
{
    s8 req[96];
    s8 numBuf[12];
    u32 reqLen;

    //Build the exact GET request first so CIPSEND's byte count is
    //always correct, whatever the key/temp/alert lengths turn out to be.
    strcpy((char*)req, "GET /update?api_key=");
    strcat((char*)req, (char*)write_key);
    strcat((char*)req, "&field1=");
    u32_to_str(temp, numBuf);
    strcat((char*)req, (char*)numBuf);
    strcat((char*)req, "&field2=");
    strcat((char*)req, alert ? "1" : "0");
    strcat((char*)req, "\r\n\r\n");
    reqLen = strlen((char*)req);

    Clear_Buffer();
    ShowCmd("AT+CIPSTART");
    U0_TxStr("AT+CIPSTART=\"TCP\",\"");
    U0_TxStr(TS_HOST);
    U0_TxStr("\",80\r\n");
    if (!waitforresponse("CONNECT", 3000))
        return 0;

    Clear_Buffer();
    u32_to_str(reqLen, numBuf);
    ShowCmd("AT+CIPSEND");
    U0_TxStr("AT+CIPSEND=");
    U0_TxU32(reqLen);
    U0_TxStr("\r\n");
    if (!waitforresponse(">", 2000))
        return 0;

    Clear_Buffer();
    ShowCmd("Sending data");
    U0_TxStr(req);
    delay_ms(5000);
    if (!waitforresponse("SEND OK", 8000))
    {
        CloseTCPConnection();
        return 0;
    }

    CloseTCPConnection();
    return 1;
}

s32 thinkspeak_read(s8 *channel_id, s8 *read_key)
{
    s8 req[96];
    s8 numBuf[12];
    u32 reqLen;
    s8 *data = NULL;
    s32 confg_setpoint = 0;

    strcpy((char*)req, "GET /channels/");
    strcat((char*)req, (char*)channel_id);
    strcat((char*)req, "/fields/1.json?api_key=");
    strcat((char*)req, (char*)read_key);
    strcat((char*)req, "&results=1\r\n\r\n");
    reqLen = strlen((char*)req);

    Clear_Buffer();
    ShowCmd("AT+CIPSTART");
    U0_TxStr("AT+CIPSTART=\"TCP\",\"");
    U0_TxStr(TS_HOST);
    U0_TxStr("\",80\r\n");
    if (!waitforresponse("CONNECT", 3000))
        return -1;

    Clear_Buffer();
    u32_to_str(reqLen, numBuf);
    ShowCmd("AT+CIPSEND");
    U0_TxStr("AT+CIPSEND=");
    U0_TxU32(reqLen);
    U0_TxStr("\r\n");
    if (!waitforresponse(">", 5000))
    {
        CloseTCPConnection();
        return -1;
    }

    Clear_Buffer();
    ShowCmd("Reading cloud");
    U0_TxStr(req);
    delay_ms(5000);

    if (waitforresponse("+IPD,", 5000))
    {
        data = strrchr((const char*)t, ':');
        if (data != NULL)
        {
            data += 2;
            confg_setpoint = 0;
            while (*data >= '0' && *data <= '9')
            {
                confg_setpoint = (confg_setpoint * 10) + (*data - '0');
                data++;
            }
            CloseTCPConnection();
            return confg_setpoint;
        }
    }
    CloseTCPConnection();
    return -1;
}
