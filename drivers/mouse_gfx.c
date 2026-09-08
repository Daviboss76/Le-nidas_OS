#include <stdint.h>
#include "vbe_glass.h"

// Variável com atributo para não gerar warning se o buffer for reservado para uso futuro
static uint32_t mouse_saved_bg[256] __attribute__((unused));

void draw_mouse_cursor(int x, int y) {
    // Desenha um cursor simples de 8x8 pixels na cor branca
    vbe_draw_rect_alpha(x, y, 8, 8, 0x00FFFFFF, 255);
}

