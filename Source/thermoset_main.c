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
#include "thermoset.h"

extern volatile u8 g_localChangeRequest;
extern s32 g_setPoint;
extern s32 g_previousSetPoint;
extern u8  g_buzzerSilenced;
extern u8  g_wasAlerting;
extern u8  wifiOK;

static void LoadSetPoints(void)
{
    u8 currentSP;
    u8 previousSP;

    currentSP = i2c_eeprom_randomread(I2C_EEPROM_SA1,
                                      EEPROM_SETPOINT_ADDR);
    previousSP = i2c_eeprom_randomread(I2C_EEPROM_SA1,
                                       EEPROM_PREVIOUS_SETPOINT_ADDR);

    if (currentSP == EEPROM_BLANK || currentSP == 0 || currentSP > MAX_SETPOINT)
    {
        g_setPoint = DEFAULT_SETPOINT;
        g_previousSetPoint = DEFAULT_SETPOINT;
        i2c_eeprom_bytewrite(I2C_EEPROM_SA1, EEPROM_SETPOINT_ADDR,
                             (u8)g_setPoint);
        i2c_eeprom_bytewrite(I2C_EEPROM_SA1,
                             EEPROM_PREVIOUS_SETPOINT_ADDR,
                             (u8)g_previousSetPoint);
    }
    else
    {
        g_setPoint = (s32)currentSP;
        if (previousSP == EEPROM_BLANK || previousSP == 0 || previousSP > MAX_SETPOINT)
            g_previousSetPoint = g_setPoint;
        else
            g_previousSetPoint = (s32)previousSP;
    }
}

static void UpdateAlertOutputs(u8 alertFlag)
{
    if (alertFlag && !g_buzzerSilenced)
    {
        Buzzer_On();
        AlertLED_On();
    }
    else
    {
        Buzzer_Off();
        AlertLED_Off();
    }
}

static void StartupScreen(void)
{
    CmdLCD(CLEAR_LCD);
    delay_ms(2);
    CmdLCD(GOTO_LINE1_POS0);
    StrLCD("ThermoSet");
    CmdLCD(GOTO_LINE2_POS0);
    StrLCD("Connect");
    delay_ms(1000);
}

int main(void)
{
    s32 temperature;
    u8  alertFlag;
    u32 lastCloudSendSec;
    u32 lastRemoteCheckSec;
    s32 remoteSP;

    UART0_Init();
    init_i2c();
    InitLCD();
    Init_KPM();
    EINT2_Setup();
    RTC_Init();

/*#if SET_INITIAL_RTC
    SetRTCTimeInfo(INIT_HOUR, INIT_MIN, INIT_SEC);
    SetRTCDateInfo(INIT_DATE, INIT_MONTH, INIT_YEAR);
    SetRTCDay(INIT_DAY);
#endif*/

    IODIR1 |= (1UL << BUZZER_PIN) | (1UL << ALERT_LED_PIN);
    Buzzer_Off();
    AlertLED_Off();

    StartupScreen();
    LoadSetPoints();

    /* Show the requested initial information before starting Wi-Fi. */
    temperature = (s32)Read_Temperature();
    alertFlag = (temperature > g_setPoint) ? 1 : 0;
    UpdateAlertOutputs(alertFlag);
    ShowTimeAndTemp(temperature, alertFlag);
    delay_ms(2500);

    /* Wi-Fi failure does not stop local temperature/alarm/menu operation. */
    LCDStatus("WiFi", "Connecting...");
    wifiOK = WiFi_Connect();
    if (!wifiOK)
    {
        LCDStatus("WiFi", "Offline - Local");
        delay_ms(1200);
    }

    lastCloudSendSec = RTC_SecondsNow();
    lastRemoteCheckSec = RTC_SecondsNow();

    while (1)
    {
        /* Local switch opens the proven keypad menu. */
        if (g_localChangeRequest)
        {
            g_localChangeRequest = 0;
            LocalSetPointChange();
        }

        temperature = (s32)Read_Temperature();
        alertFlag = (temperature > g_setPoint) ? 1 : 0;

        /* A new alert episode is posted immediately. */
        if (alertFlag && !g_wasAlerting)
        {
            g_buzzerSilenced = 0;
            UpdateAlertOutputs(1);
            if (wifiOK)
            {
                lastCloudSendSec = RTC_SecondsNow();
                if (!thinkspeak_write(TS1_WRITE_API_KEY,
                                      (u32)temperature, 1))
                    ShowCloudWriteFail();
            }
        }
        g_wasAlerting = alertFlag;

        UpdateAlertOutputs(alertFlag);

        /* Any keypad press while alarming silences the buzzer only. */
        if (alertFlag && !g_buzzerSilenced && colscan() == 0)
        {
            g_buzzerSilenced = 1;
            Buzzer_Off();
            AlertLED_On();
        }

        ShowTimeAndTemp(temperature, alertFlag);

        if (wifiOK &&
            RTC_ElapsedSec(lastCloudSendSec) >= CLOUD_SEND_INTERVAL_SEC)
        {
            lastCloudSendSec = RTC_SecondsNow();
            if (!thinkspeak_write(TS1_WRITE_API_KEY,
                                  (u32)temperature, alertFlag))
                ShowCloudWriteFail();
        }

        if (wifiOK &&
            RTC_ElapsedSec(lastRemoteCheckSec) >= REMOTE_CHECK_INTERVAL_SEC)
        {
            lastRemoteCheckSec = RTC_SecondsNow();
            ShowCloudCheckStatus();
            remoteSP = thinkspeak_read(TS2_CHANNEL_ID, TS2_READ_API_KEY);

            if (remoteSP < 1 || remoteSP > MAX_SETPOINT)
            {
                ShowCloudReadFail();
            }
            else if (remoteSP == g_setPoint)
            {
                ShowCloudNoChange();
                delay_ms(500);
            }
            else
            {
                ShowCloudUpdateStatus();
                g_previousSetPoint = g_setPoint;
                g_setPoint = remoteSP;

                i2c_eeprom_bytewrite(I2C_EEPROM_SA1,
                                     EEPROM_PREVIOUS_SETPOINT_ADDR,
                                     (u8)g_previousSetPoint);
                i2c_eeprom_bytewrite(I2C_EEPROM_SA1,
                                     EEPROM_SETPOINT_ADDR,
                                     (u8)g_setPoint);

                /* Force a fresh alert-state evaluation without reset. */
                g_buzzerSilenced = 0;
                g_wasAlerting = 0;

                ShowCloudUpdateOK();
                delay_ms(900);
            }
        }

        delay_ms(1000);
    }
}
