#define EEPROM_SETPOINT_ADDR            0x0000
#define EEPROM_PREVIOUS_SETPOINT_ADDR   0x0001
#define DEFAULT_SETPOINT                35
#define MAX_SETPOINT                    150
#define EEPROM_BLANK                    0xFF

#define CLOUD_SEND_INTERVAL_SEC         150   /* 2.5 min heartbeat */
#define REMOTE_CHECK_INTERVAL_SEC       90    /* 1.5 min cloud SP poll */

#define BUZZER_PIN       24
#define ALERT_LED_PIN    25
#define BUZZER_ACTIVE_HIGH 1

#define EINT2_PIN_FUNC   (3UL << 14)       /* P0.7 = EINT2 */
#define EINT2_VIC_CHNO   16

#define READNUM_CANCEL        0xFFFFFFFF
#define READNUM_EMPTY_ENTER   0xFFFFFFFE
#define READNUM_NEXT          0xFFFFFFFD
#define READNUM_PREV          0xFFFFFFFC

#define SET_INITIAL_RTC  1
#define INIT_HOUR        10
#define INIT_MIN         0
#define INIT_SEC         0
#define INIT_DATE        1
#define INIT_MONTH       1
#define INIT_YEAR        2026
#define INIT_DAY         MON
