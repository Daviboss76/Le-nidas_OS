// vga.h
#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_clear(void);
void vga_putchar(char c, uint8_t color);
void vga_puts(const char *str);
void vga_backspace(void);

#endif

