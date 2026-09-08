#include "window_manager.h"
#include "vbe_glass.h"

static window_t windows[MAX_WINDOWS];
static int window_count = 0;
static int active_drag_id = -1;

void window_manager_init(void) {
    window_count = 0;
    active_drag_id = -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].active = 0;
        windows[i].is_dragging = 0;
    }
}

void window_focus(int window_id) {
    if (window_id < 0 || window_id >= MAX_WINDOWS) return;
    for (int i = 0; i < window_count; i++) {
        if (windows[i].id == window_id) {
            windows[i].active = 1;
        }
    }
}

int window_create(int x, int y, int w, int h, const char* title, uint32_t bg_color, uint8_t alpha) {
    if (window_count >= MAX_WINDOWS) return -1;

    int id = window_count;
    windows[id].id = id;
    windows[id].x = x;
    windows[id].y = y;
    windows[id].width = w;
    windows[id].height = h;
    windows[id].title = title;
    windows[id].bg_color = bg_color;
    windows[id].alpha = alpha;
    windows[id].flags = 0;
    windows[id].active = 1;
    windows[id].is_dragging = 0;
    windows[id].drag_offset_x = 0;
    windows[id].drag_offset_y = 0;

    window_focus(id);
    window_count++;
    return id;
}

void window_manager_draw_all(void) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].active) {
            vbe_render_glass_window(&windows[i]);
        }
    }
}

void window_manager_handle_mouse(int mouse_x, int mouse_y, uint8_t buttons) {
    uint8_t left_click = buttons & 0x01;

    if (active_drag_id != -1) {
        if (!left_click) {
            windows[active_drag_id].is_dragging = 0;
            active_drag_id = -1;
        } else {
            window_t* win = &windows[active_drag_id];
            win->x = mouse_x - win->drag_offset_x;
            win->y = mouse_y - win->drag_offset_y;
        }
        return;
    }

    if (left_click) {
        for (int i = window_count - 1; i >= 0; i--) {
            window_t* win = &windows[i];
            if (!win->active) continue;

            if (mouse_x >= win->x && mouse_x <= win->x + win->width &&
                mouse_y >= win->y && mouse_y <= win->y + win->height) {
                
                window_focus(win->id);
                win->is_dragging = 1;
                win->drag_offset_x = mouse_x - win->x;
                win->drag_offset_y = mouse_y - win->y;
                active_drag_id = win->id;
                break;
            }
        }
    }
}

