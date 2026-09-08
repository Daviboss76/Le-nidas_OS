#include "vbe_glass.h"
#include <stdint.h>

static uint32_t* vbe_framebuffer = (uint32_t*)0;

void vbe_init(uint32_t lfb_addr) {
    vbe_framebuffer = (uint32_t*)lfb_addr;
}

void vbe_flip(void) {
    // Buffer swap para visualização direta em VBE
}

uint32_t blend_pixel(uint32_t bg, uint32_t fg, uint8_t alpha) {
    if (alpha == 255) return fg;
    if (alpha == 0) return bg;

    uint32_t rb = ((fg & 0x00FF00FF) * alpha + (bg & 0x00FF00FF) * (255 - alpha)) >> 8;
    uint32_t g  = ((fg & 0x0000FF00) * alpha + (bg & 0x0000FF00) * (255 - alpha)) >> 8;

    return (rb & 0x00FF00FF) | (g & 0x0000FF00);
}

void vbe_draw_rect_alpha(int x, int y, int w, int h, uint32_t color, uint8_t alpha) {
    if (!vbe_framebuffer) return;

    for (int i = y; i < y + h; i++) {
        if (i < 0 || i >= VBE_HEIGHT) continue;
        for (int j = x; j < x + w; j++) {
            if (j < 0 || j >= VBE_WIDTH) continue;
            int index = i * VBE_WIDTH + j;
            vbe_framebuffer[index] = blend_pixel(vbe_framebuffer[index], color, alpha);
        }
    }
}

void vbe_render_glass_window(const window_t* win) {
    if (!win) return;

    (void)win->title;
    vbe_draw_rect_alpha(win->x, win->y, win->width, win->height, win->bg_color, win->alpha);
}

