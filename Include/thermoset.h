#include "types.h"

void eint2_isr(void) __irq;
void EINT2_Setup(void);

u8 wifi_step(s8 *cmd, s8 *lcdLabel, s8 *expect, u32 timeout_ms);
u8 WiFi_Connect(void);

void LocalSetPointChange(void);
void Enter_ConfigMenu(void);
void Configure_SetPoint(void);
void Configure_Time(void);
void Configure_Date(void);
void Show_Previous_SetPoint(void);

u32 RTC_SecondsNow(void);
u32 RTC_ElapsedSec(u32 lastSec);
void ShowTimeAndTemp(s32 temperature, u8 alertFlag);
void ShowDate(void);

void ShowCloudCheckStatus(void);
void ShowCloudUpdateStatus(void);
void ShowCloudNoChange(void);
void ShowCloudUpdateOK(void);
void ShowCloudReadFail(void);
void ShowCloudWriteFail(void);

void Buzzer_On(void);
void Buzzer_Off(void);
void AlertLED_On(void);
void AlertLED_Off(void);

void LCDStatus(s8 *line1, s8 *line2);
void SaveSetPoints(void);
void ApplyNewSetPoint(s32 newSetPoint);

extern s32 g_setPoint;
extern s32 g_previousSetPoint;
extern u8 g_buzzerSilenced;
extern u8 g_wasAlerting;
