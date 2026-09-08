#ifndef VBE_GLASS_H
#define VBE_GLASS_H

#include <stdint.h>
#include "window_manager.h"

#define VBE_WIDTH  1024
#define VBE_HEIGHT 768

void vbe_init(uint32_t lfb_addr);
void vbe_flip(void);
uint32_t blend_pixel(uint32_t bg, uint32_t fg, uint8_t alpha);
void vbe_draw_rect_alpha(int x, int y, int w, int h, uint32_t color, uint8_t alpha);
void vbe_render_glass_window(const window_t* win);

#endif

