#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <stdint.h>

#define MAX_WINDOWS 16

typedef struct {
    int id;
    int x;
    int y;
    int width;
    int height;
    const char* title;
    uint32_t bg_color;
    uint8_t alpha;
    uint8_t flags;
    uint8_t active;
    uint8_t is_dragging;
    int drag_offset_x;
    int drag_offset_y;
} window_t;

void window_manager_init(void);
void window_manager_draw_all(void);
int window_create(int x, int y, int w, int h, const char* title, uint32_t bg_color, uint8_t alpha);
void window_focus(int window_id);
void window_manager_handle_mouse(int mouse_x, int mouse_y, uint8_t buttons);

#endif

