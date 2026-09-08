#include "vbe_glass.h"
#include "window_manager.h"

static uint8_t start_menu_open = 0;
static char search_query[32] = "";
static int search_cursor_pos = 0;

typedef struct {
    const char* name;
    const char* path;
    uint32_t cluster_id;
} fs_app_t;

static fs_app_t partition_apps[] = {
    {"Terminal.elf", "/bin/terminal", 0x102A},
    {"Editor.elf",   "/bin/editor",   0x104B},
    {"Explorer.elf", "/bin/files",    0x108C},
    {"Settings.elf", "/bin/settings", 0x1100}
};
static int partition_app_count = 4;

void gui_desktop_init(uint32_t lfb_addr) {
    vbe_init(lfb_addr);
    start_menu_open = 0;
    search_cursor_pos = 0;
    search_query[0] = '\0';
}

void gui_desktop_render(void) {
    for (int y = 0; y < VBE_HEIGHT - 48; y++) {
        uint32_t color = blend_pixel(0x000F172A, 0x001E1B4B, (y * 255) / VBE_HEIGHT);
        vbe_draw_rect_alpha(0, y, VBE_WIDTH, 1, color, 255);
    }

    window_manager_draw_all();

    // Barra de tarefas
    vbe_draw_rect_alpha(0, VBE_HEIGHT - 48, VBE_WIDTH, 48, 0x000B0F19, 210);

    uint32_t start_btn_bg = start_menu_open ? 0x0038BDF8 : 0x001E293B;
    vbe_draw_rect_alpha(10, VBE_HEIGHT - 42, 92, 36, start_btn_bg, 230);

    if (start_menu_open) {
        int menu_w = 300;
        int menu_h = 360;
        int menu_x = 10;
        int menu_y = VBE_HEIGHT - 48 - menu_h - 10;

        vbe_draw_rect_alpha(menu_x, menu_y, menu_w, menu_h, 0x000F172A, 240);
        vbe_draw_rect_alpha(menu_x, menu_y, menu_w, 1, 0x0038BDF8, 255);

        // Uso das variáveis para evitar warnings
        (void)search_query;
        (void)search_cursor_pos;
        vbe_draw_rect_alpha(menu_x + 12, menu_y + 14, menu_w - 24, 32, 0x001E293B, 255);

        int list_y = menu_y + 60;
        for (int i = 0; i < partition_app_count; i++) {
            if (partition_apps[i].name[0] != '\0') {
                vbe_draw_rect_alpha(menu_x + 12, list_y, menu_w - 24, 34, 0x001E293B, 100);
            }
            list_y += 40;
        }
    }

    vbe_flip();
}

void gui_desktop_handle_mouse_click(int mouse_x, int mouse_y, uint8_t left_click) {
    if (!left_click) return;

    if (mouse_x >= 10 && mouse_x <= 102 && mouse_y >= VBE_HEIGHT - 42 && mouse_y <= VBE_HEIGHT - 6) {
        start_menu_open = !start_menu_open;
        return;
    }

    if (start_menu_open) {
        int menu_w = 300;
        int menu_h = 360;
        int menu_x = 10;
        int menu_y = VBE_HEIGHT - 48 - menu_h - 10;

        if (mouse_x < menu_x || mouse_x > menu_x + menu_w || mouse_y < menu_y || mouse_y > menu_y + menu_h) {
            start_menu_open = 0;
        }
    }
}

