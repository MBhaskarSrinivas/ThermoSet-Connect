#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "string.h"
#include "delay.h"
#include "uart.h"
#include "uart_defines.h"
#include "lcd.h"
#include "lcd_defines.h"
#include "kpm.h"
#include "adc.h"
#include "lm35.h"
#include "i2c.h"
#include "i2c_eeprom.h"
#include "i2c_eeprom_defines.h"
#include "rtc.h"
#include "rtc_defines.h"
#include "esp01.h"
#include "esp01_defines.h"
#include "thermoset_defines.h"

volatile u8 g_localChangeRequest = 0;
u8  g_wasAlerting = 0;
s32 g_setPoint;
s32 g_previousSetPoint;
u8  g_buzzerSilenced = 0;
u8  wifiOK;

void eint2_isr(void) __irq;
void LCDStatus(s8 *line1, s8 *line2)
{
    CmdLCD(CLEAR_LCD);
    delay_ms(2);
    CmdLCD(GOTO_LINE1_POS0);
    StrLCD(line1);
    if (line2 != NULL)
    {
        CmdLCD(GOTO_LINE2_POS0);
        StrLCD(line2);
    }
}

void SaveSetPoints(void)
{
    i2c_eeprom_bytewrite(I2C_EEPROM_SA1, EEPROM_SETPOINT_ADDR,
                         (u8)g_setPoint);
    i2c_eeprom_bytewrite(I2C_EEPROM_SA1, EEPROM_PREVIOUS_SETPOINT_ADDR,
                         (u8)g_previousSetPoint);
}

void ApplyNewSetPoint(s32 newSetPoint)
{
    if (newSetPoint == g_setPoint)
        return;

    g_previousSetPoint = g_setPoint;
    g_setPoint = newSetPoint;
    SaveSetPoints();

    /* Re-evaluate alert state immediately in the next main-loop pass. */
    g_buzzerSilenced = 0;
    g_wasAlerting = 0;
}

void EINT2_Setup(void)
{
    PINSEL0 &= ~(3UL << 14);
    PINSEL0 |= EINT2_PIN_FUNC;

    VICIntEnable = 1UL << EINT2_VIC_CHNO;
    VICVectCntl1 = (1UL << 5) | EINT2_VIC_CHNO;
    VICVectAddr1 = (u32)eint2_isr;

    EXTMODE |= 1UL << 2;
}

void eint2_isr(void) __irq
{
    g_localChangeRequest = 1;
    VICVectAddr = 0;
    EXTINT = 1UL << 2;
}

u8 wifi_step(s8 *cmd, s8 *lcdLabel, s8 *expect, u32 timeout_ms)
{
    u8 tries;

    for (tries = 0; tries < 5; tries++)
    {
        Clear_Buffer();
        LCDStatus(lcdLabel, "Waiting...");
        U0_TxStr(cmd);
        if (waitforresponse(expect, timeout_ms))
            return 1;
    }
    return 0;
}

u8 WiFi_Connect(void)
{
    s8 joinCmd[80];

    LCDStatus("WiFi", "Disconnecting");
    wifi_step("AT+CWQAP\r\n", "AT+CWQAP", "WIFI DISCONNECT", 2000);

    LCDStatus("WiFi", "Checking ESP");
    if (!wifi_step("AT\r\n", "AT", "OK", 1500))
        return 0;

    if (!wifi_step("ATE0\r\n", "ATE0", "OK", 1500))
        return 0;
    if (!wifi_step("AT+CIPMUX=0\r\n", "CIPMUX=0", "OK", 1500))
        return 0;
    if (!wifi_step("AT+CWMODE=1\r\n", "CWMODE=1", "OK", 1500))
        return 0;

    strcpy((char *)joinCmd, "AT+CWJAP=\"");
    strcat((char *)joinCmd, WIFI_SSID);
    strcat((char *)joinCmd, "\",\"");
    strcat((char *)joinCmd, WIFI_PASSWORD);
    strcat((char *)joinCmd, "\"\r\n");

    if (!wifi_step(joinCmd, "Joining WiFi", "WIFI CONNECTED", 3000))
        return 0;

    Clear_Buffer();
    if (!waitforresponse("WIFI GOT IP", 1000))
        return 0;

    Clear_Buffer();
    //if (!waitforresponse("OK", 1000))
      //  return 0;

    LCDStatus("WiFi Connected", "Cloud Ready");
    delay_ms(1000);
    return 1;
}

void Configure_SetPoint(void)
{
    u32 newVal;

    while (1)
    {	CmdLCD(CLEAR_LCD);
        StrLCD("New SP:");
		CmdLCD(GOTO_LINE2_POS0);
        newVal = ReadNUM();

        if (newVal == READNUM_CANCEL)
        {
            LCDStatus("Setpoint", "Cancelled");
            delay_ms(700);
            return;
        }
        if (newVal == READNUM_EMPTY_ENTER)
            return;
        if (newVal == READNUM_NEXT || newVal == READNUM_PREV)
            continue;

        if (newVal == 0 || newVal > MAX_SETPOINT)
        {
            LCDStatus("Invalid Input", "Range 1-150");
            delay_ms(700);
            continue;
        }

        ApplyNewSetPoint((s32)newVal);
        LCDStatus("Setpoint", "Saved to EEPROM");
        delay_ms(900);
		if (wifiOK)
    	{
        	CmdLCD(CLEAR_LCD);
        	StrLCD("Updating Cloud");
        	CmdLCD(GOTO_LINE2_POS0);
        	if (thinkspeak_write(TS2_WRITE_API_KEY,(u32)g_setPoint, 0))
        	{
         	   StrLCD("Cloud Updated");
        	}
        	else
        	{	
           	 	StrLCD("Cloud Update Fail");
      		}
        	delay_ms(1000);
		}
        return;
    }
}

void Configure_Time(void)
{
    s32 hour, min, sec;
    u32 temp;

    GetRTCTimeInfo(&hour, &min, &sec);

TIME_HOUR:
    CmdLCD(CLEAR_LCD);
	StrLCD("Hour(0-23):");
	CmdLCD(GOTO_LINE2_POS0);
    temp = ReadNUM();
    if (temp == READNUM_CANCEL) goto TIME_CANCEL;
    if (temp == READNUM_NEXT) goto TIME_MIN;
    if (temp == READNUM_PREV) goto TIME_HOUR;
    if (temp == READNUM_EMPTY_ENTER) goto TIME_MIN;
    if (temp > 23) goto TIME_BAD_HOUR;
    hour = (s32)temp;

TIME_MIN:
    CmdLCD(CLEAR_LCD);
	StrLCD("Min(0-59):");
	CmdLCD(GOTO_LINE2_POS0);
    temp = ReadNUM();
    if (temp == READNUM_CANCEL) goto TIME_CANCEL;
    if (temp == READNUM_NEXT) goto TIME_SEC;
    if (temp == READNUM_PREV) goto TIME_HOUR;
    if (temp == READNUM_EMPTY_ENTER) goto TIME_SEC;
    if (temp > 59) goto TIME_BAD_MIN;
    min = (s32)temp;

TIME_SEC:
    CmdLCD(CLEAR_LCD);
	StrLCD("Sec(0-59):");
	CmdLCD(GOTO_LINE2_POS0);
    temp = ReadNUM();
    if (temp == READNUM_CANCEL) goto TIME_CANCEL;
    if (temp == READNUM_PREV) goto TIME_MIN;
    if (temp == READNUM_NEXT) goto TIME_SEC;
    if (temp == READNUM_EMPTY_ENTER) goto TIME_SET;
    if (temp > 59) goto TIME_BAD_SEC;
    sec = (s32)temp;

TIME_SET:
    SetRTCTimeInfo((u32)hour, (u32)min, (u32)sec);
    LCDStatus("Time", "Updated");
    delay_ms(800);
    return;

TIME_BAD_HOUR:
    LCDStatus("Invalid Input", "Hour 0-23");
    delay_ms(700);
    goto TIME_HOUR;
TIME_BAD_MIN:
    LCDStatus("Invalid Input", "Min 0-59");
    delay_ms(700);
    goto TIME_MIN;
TIME_BAD_SEC:
    LCDStatus("Invalid Input", "Sec 0-59");
    delay_ms(700);
    goto TIME_SEC;
TIME_CANCEL:
    LCDStatus("Time", "Updation Failed");
    delay_ms(700);
}

void Configure_Date(void)
{
    s32 dd, mon, yyyy;
    u32 temp;

    GetRTCDateInfo(&dd, &mon, &yyyy);

DATE_DAY:
    CmdLCD(CLEAR_LCD);
	StrLCD("Date(1-31):");
    CmdLCD(GOTO_LINE2_POS0);
	temp = ReadNUM();
    if (temp == READNUM_CANCEL) goto DATE_CANCEL;
    if (temp == READNUM_NEXT) goto DATE_MONTH;
    if (temp == READNUM_PREV) goto DATE_DAY;
    if (temp == READNUM_EMPTY_ENTER) goto DATE_MONTH;
    if (temp == 0 || temp > 31) goto DATE_BAD_DAY;
    dd = (s32)temp;

DATE_MONTH:
    CmdLCD(CLEAR_LCD);
	StrLCD("Month(1-12):");
    CmdLCD(GOTO_LINE2_POS0);
	temp = ReadNUM();
    if (temp == READNUM_CANCEL) goto DATE_CANCEL;
    if (temp == READNUM_NEXT) goto DATE_YEAR;
    if (temp == READNUM_PREV) goto DATE_DAY;
    if (temp == READNUM_EMPTY_ENTER) goto DATE_YEAR;
    if (temp == 0 || temp > 12) goto DATE_BAD_MONTH;
    mon = (s32)temp;

DATE_YEAR:
    CmdLCD(CLEAR_LCD);
	StrLCD("Year:");
	CmdLCD(GOTO_LINE2_POS0);
    temp = ReadNUM();
    if (temp == READNUM_CANCEL) goto DATE_CANCEL;
    if (temp == READNUM_PREV) goto DATE_MONTH;
    if (temp == READNUM_NEXT || temp == READNUM_EMPTY_ENTER) goto DATE_SET;
    if (temp < 2000) goto DATE_BAD_YEAR;
    yyyy = (s32)temp;

DATE_SET:
    SetRTCDateInfo((u32)dd, (u32)mon, (u32)yyyy);
    LCDStatus("Date", "Updated");
    delay_ms(800);
    return;

DATE_BAD_DAY:
    LCDStatus("Invalid Input", "Date 1-31");
    delay_ms(700);
    goto DATE_DAY;
DATE_BAD_MONTH:
    LCDStatus("Invalid Input", "Month 1-12");
    delay_ms(700);
    goto DATE_MONTH;
DATE_BAD_YEAR:
    LCDStatus("Invalid Input", "Year >=2000");
    delay_ms(700);
    goto DATE_YEAR;
DATE_CANCEL:
    LCDStatus("Date", "Updation Failed");
    delay_ms(700);
}

void Show_Previous_SetPoint(void)
{
    LCDStatus("Previous SP", "Value:");
    CmdLCD(GOTO_LINE2_POS0);
    S32LCD(g_previousSetPoint);
    StrLCD(" C");
    //delay_ms(12000);
	while(keyscan()!='C');
}

void Enter_ConfigMenu(void)
{
    u32 choice;

MENU_PAGE1:
    LCDStatus("1:Time 2:Date", "3:SP N:Next");
    choice = keyscan();

    if (choice == '1')
    {
        Configure_Time();
        goto MENU_PAGE1;
    }
    if (choice == '2')
    {
        Configure_Date();
        goto MENU_PAGE1;
    }
    if (choice == '3')
    {
        Configure_SetPoint();

        goto MENU_PAGE1;
    }
    if (choice == 'N')
        goto MENU_PAGE2;
    if (choice == 'C')
        return;
	goto MENU_PAGE1;

MENU_PAGE2:
    LCDStatus("4:Prev SP", "P:Prev C:Exit");
    choice = keyscan();

    if (choice == '4')
    {
        Show_Previous_SetPoint();
        goto MENU_PAGE2;
    }
    if (choice == 'P')
        goto MENU_PAGE1;
    if (choice == 'C')
        return;
    goto MENU_PAGE2;
}

void LocalSetPointChange(void)
{
    /* Kept as the existing interrupt entry point; the tested menu now
       handles set point, time, date and previous-setpoint functions. */
    Enter_ConfigMenu();
}

u32 RTC_SecondsNow(void)
{
    s32 hh, mm, ss;
    GetRTCTimeInfo(&hh, &mm, &ss);
    return (u32)hh * 3600UL + (u32)mm * 60UL + (u32)ss;
}

u32 RTC_ElapsedSec(u32 lastSec)
{
    u32 now = RTC_SecondsNow();
    if (now >= lastSec)
        return now - lastSec;
    return (86400UL - lastSec) + now;
}

void ShowTimeAndTemp(s32 temperature, u8 alertFlag)
{
    s32 hh, mm, ss;

    CmdLCD(CLEAR_LCD);
    delay_ms(1);
    GetRTCTimeInfo(&hh, &mm, &ss);
    DisplayRTCTime((u32)hh, (u32)mm, (u32)ss);
    StrLCD(" SP:");
    S32LCD(g_setPoint);

    CmdLCD(GOTO_LINE2_POS0);
    StrLCD("T:");
    S32LCD(temperature);
    StrLCD("C ");
    if (alertFlag)
        StrLCD(g_buzzerSilenced ? "ALRT(sil)" : "ALERT!");
    else
        StrLCD("OK");
}

void ShowDate(void)
{
    s32 dd, mon, yyyy;
    GetRTCDateInfo(&dd, &mon, &yyyy);
    CmdLCD(CLEAR_LCD);
    delay_ms(1);
    DisplayRTCDate((u32)dd, (u32)mon, (u32)yyyy);
}

void ShowCloudCheckStatus(void)
{
    LCDStatus("Cloud", "Checking SP...");
}

void ShowCloudUpdateStatus(void)
{
    LCDStatus("Cloud", "Updating SP...");
}

void ShowCloudNoChange(void)
{
    LCDStatus("Cloud", "SP unchanged");
}

void ShowCloudUpdateOK(void)
{
    LCDStatus("Cloud", "SP update OK");
}

void ShowCloudReadFail(void)
{
    LCDStatus("Cloud", "Read failed");
    delay_ms(700);
}

void ShowCloudWriteFail(void)
{
    LCDStatus("Cloud", "Write failed");
    delay_ms(700);
}

void Buzzer_On(void)
{
#if BUZZER_ACTIVE_HIGH
    IOSET1 = (1UL << BUZZER_PIN);
#else
    IOCLR1 = (1UL << BUZZER_PIN);
#endif
}

void Buzzer_Off(void)
{
#if BUZZER_ACTIVE_HIGH
    IOCLR1 = (1UL << BUZZER_PIN);
#else
    IOSET1 = (1UL << BUZZER_PIN);
#endif
}

void AlertLED_On(void)
{
    IOSET1 = (1UL << ALERT_LED_PIN);
}

void AlertLED_Off(void)
{
    IOCLR1 = (1UL << ALERT_LED_PIN);
}
