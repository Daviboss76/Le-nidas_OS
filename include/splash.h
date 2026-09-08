#ifndef SPLASH_H
#define SPLASH_H

#include <stdint.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200
#define VGA_GFX_MEM   ((uint8_t*)0xA0000)

// Inicializa o modo gráfico 320x200 e exibe a tela de boot
void splash_show(void);

// Limpa a tela gráfica preenchendo com uma cor específica
void splash_clear(uint8_t color);

// Plota um pixel individual na tela
void splash_draw_pixel(int x, int y, uint8_t color);

#endif

