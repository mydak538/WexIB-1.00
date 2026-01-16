#ifndef WATCH_H
#define WATCH_H

#include <stdint.h>

// Регистры CMOS для времени
#define CMOS_SECOND      0x00
#define CMOS_MINUTE      0x02
#define CMOS_HOUR        0x04
#define CMOS_WEEKDAY     0x06
#define CMOS_DAY         0x07
#define CMOS_MONTH       0x08
#define CMOS_YEAR        0x09
#define CMOS_CENTURY     0x32
#define CMOS_STATUS_A    0x0A
#define CMOS_STATUS_B    0x0B

// Структура для хранения времени
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday;
} cmos_time_t;

// Прототипы функций
void cmos_read_time(cmos_time_t *time);
void cmos_display_time(void);
void cmos_display_date(void);
uint8_t cmos_bcd_to_bin(uint8_t bcd);
void cmos_wait_for_update(void);
void cmos_update_display(void);
void watch_init(void);

#endif // WATCH_H
