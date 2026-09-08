#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} rtc_time_t;

// Lê a data e hora do chip CMOS/RTC
void rtc_read_datetime(rtc_time_t *time);

// Converte a estrutura de data/hora em uma string formatada (ex: "DD/MM/AAAA HH:MM:SS")
void rtc_format_datetime(const rtc_time_t *time, char *out_str);

#endif

