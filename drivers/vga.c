// vga.c
#include "vga.h"

#define VIDEO_MEM ((volatile uint16_t*)0xB8000)
#define WIDTH 80
#define HEIGHT 25

static uint32_t cursor_pos = 0;

static void scroll(void) {
    if (cursor_pos >= WIDTH * HEIGHT) {
        for (int i = 0; i < WIDTH * (HEIGHT - 1); i++) {
            VIDEO_MEM[i] = VIDEO_MEM[i + WIDTH];
        }
        for (int i = WIDTH * (HEIGHT - 1); i < WIDTH * HEIGHT; i++) {
            VIDEO_MEM[i] = (0x0F << 8) | ' ';
        }
        cursor_pos = WIDTH * (HEIGHT - 1);
    }
}

void vga_clear(void) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        VIDEO_MEM[i] = (0x0F << 8) | ' ';
    }
    cursor_pos = 0;
}

void vga_putchar(char c, uint8_t color) {
    if (c == '\n') {
        cursor_pos += WIDTH - (cursor_pos % WIDTH);
    } else {
        VIDEO_MEM[cursor_pos++] = (color << 8) | c;
    }
    scroll();
}

void vga_puts(const char *str) {
    while (*str) vga_putchar(*str++, 0x0F);
}

void vga_backspace(void) {
    if (cursor_pos > 0) {
        cursor_pos--;
        VIDEO_MEM[cursor_pos] = (0x0F << 8) | ' ';
    }
}

