#include "RTC.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// Funções de I/O por porta
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Lê um registrador do CMOS
static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

// Verifica se o RTC está atualizando os dados no momento
static int get_update_in_progress_flag(void) {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

// Função auxiliar para converter 2 dígitos para texto no formato "00"
static void u8_to_2digits(uint8_t val, char *str) {
    str[0] = '0' + (val / 10);
    str[1] = '0' + (val % 10);
}

void rtc_read_datetime(rtc_time_t *time) {
    // Aguarda se o RTC estiver no meio de uma atualização
    while (get_update_in_progress_flag());

    time->second = get_rtc_register(0x00);
    time->minute = get_rtc_register(0x02);
    time->hour   = get_rtc_register(0x04);
    time->day    = get_rtc_register(0x07);
    time->month  = get_rtc_register(0x08);
    time->year   = get_rtc_register(0x09);

    uint8_t registerB = get_rtc_register(0x0B);

    // Se o formato dos registradores for BCD, converte para decimal puro
    if (!(registerB & 0x04)) {
        time->second = (time->second & 0x0F) + ((time->second / 16) * 10);
        time->minute = (time->minute & 0x0F) + ((time->minute / 16) * 10);
        time->hour   = ((time->hour & 0x0F) + (((time->hour & 0x70) / 16) * 10)) | (time->hour & 0x80);
        time->day    = (time->day & 0x0F) + ((time->day / 16) * 10);
        time->month  = (time->month & 0x0F) + ((time->month / 16) * 10);
        time->year   = (time->year & 0x0F) + ((time->year / 16) * 10);
    }

    time->year += 2000;
}

void rtc_format_datetime(const rtc_time_t *time, char *out_str) {
    // Preenche a string no formato DD/MM/AAAA HH:MM:SS
    u8_to_2digits(time->day, &out_str[0]);
    out_str[2] = '/';
    u8_to_2digits(time->month, &out_str[3]);
    out_str[5] = '/';
    
    // Ano (20XX)
    out_str[6] = '2';
    out_str[7] = '0';
    u8_to_2digits((uint8_t)(time->year % 100), &out_str[8]);
    out_str[10] = ' ';

    // Hora
    u8_to_2digits(time->hour, &out_str[11]);
    out_str[13] = ':';
    u8_to_2digits(time->minute, &out_str[14]);
    out_str[16] = ':';
    u8_to_2digits(time->second, &out_str[17]);
    out_str[19] = '\0';
}

