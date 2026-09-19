#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "leofiles.h"
#include "RTC.h"

extern char kbd_getchar(void);
extern void shell_init(void);
// Supõe que o seu shell tem uma função que roda o loop dele e retorna quando o usuário digita 'exit'
// Se o seu shell atual chama shell_init() e já entra em loop, veja a nota abaixo.
extern void shell_run(void); 
extern void basic_rodar_arquivo(const char *nome);

#define LARGURA 80
#define ALTURA 25
#define MEM_VGA ((volatile uint16_t*)0xB8000)

#define COR_DESKTOP     0x1F  
#define COR_BARRA       0x30  
#define COR_JANELA_HDR  0x70  
#define COR_JANELA_BODY 0x1E  
#define COR_MENU_SEL    0x2F  
#define COR_SOMBRA      0x08  

static uint16_t buffer_tela[LARGURA * ALTURA];

static int menu_aberto = 0;
static int item_selecionado = 0;
static int wallpaper_index = 0;
static const char *wallpapers[] = {"padrao.img", "praise.img", "retro.img"};

static void put_char_buffer(int x, int y, char c, uint8_t cor) {
    if (x >= 0 && x < LARGURA && y >= 0 && y < ALTURA) {
        buffer_tela[y * LARGURA + x] = (uint16_t)c | ((uint16_t)cor << 8);
    }
}

static void put_string_buffer(int x, int y, const char *str, uint8_t cor) {
    while (*str && x < LARGURA) {
        put_char_buffer(x, y, *str, cor);
        str++;
        x++;
    }
}

static void desenhar_janela(int x, int y, int w, int h, const char *titulo, uint8_t cor_body) {
    for (int i = 1; i <= h; i++) {
        for (int j = 2; j <= 2; j++) {
            int sx = x + w + j - 2;
            int sy = y + i;
            if (sx < LARGURA && sy < ALTURA) {
                uint16_t atual = buffer_tela[sy * LARGURA + sx];
                buffer_tela[sy * LARGURA + sx] = (atual & 0x00FF) | (COR_SOMBRA << 8);
            }
        }
    }

    for (int i = 0; i < w; i++) put_char_buffer(x + i, y, ' ', COR_JANELA_HDR);
    put_string_buffer(x + 2, y, titulo, COR_JANELA_HDR);

    for (int line = 1; line < h; line++) {
        for (int col = 0; col < w; col++) {
            if (col == 0 || col == w - 1) {
                put_char_buffer(x + col, y + line, '|', cor_body);
            } else if (line == h - 1) {
                put_char_buffer(x + col, y + line, '-', cor_body);
            } else {
                put_char_buffer(x + col, y + line, ' ', cor_body);
            }
        }
    }
}

static void tui_flush(void) {
    for (int i = 0; i < LARGURA * ALTURA; i++) {
        MEM_VGA[i] = buffer_tela[i];
    }
}

static void tui_desenhar_desktop(void) {
    for (int i = 0; i < LARGURA * ALTURA; i++) {
        buffer_tela[i] = (uint16_t)' ' | (COR_DESKTOP << 8);
    }

    put_string_buffer(26, 10, "=== LE-NIDAS OS DESKTOP ===", COR_DESKTOP);
    put_string_buffer(22, 12, "Sistema Operacional 32-bit em Modo Texto", COR_DESKTOP);
    put_string_buffer(20, 22, "[M] Menu  |  [W/S] Navegar  |  [ENTER] Executar", COR_DESKTOP);
}

static void tui_desenhar_barra(void) {
    rtc_time_t hora;
    char hora_str[32];
    rtc_read_datetime(&hora);
    rtc_format_datetime(&hora, hora_str);

    for (int i = 0; i < LARGURA; i++) {
        put_char_buffer(i, 24, ' ', COR_BARRA);
    }

    if (menu_aberto) {
        put_string_buffer(1, 24, " [ INICIAR (ABERTO) ] ", COR_MENU_SEL);
    } else {
        put_string_buffer(1, 24, " [ INICIAR ] ", COR_BARRA);
    }

    put_string_buffer(60, 24, hora_str, COR_BARRA);
}

static void tui_desenhar_menu(void) {
    if (!menu_aberto) return;

    int x = 2, y = 14, w = 34, h = 9;
    desenhar_janela(x, y, w, h, " Menu Iniciar ", COR_JANELA_BODY);

    uint8_t cor_opt0 = (item_selecionado == 0) ? COR_MENU_SEL : COR_JANELA_BODY;
    uint8_t cor_opt1 = (item_selecionado == 1) ? COR_MENU_SEL : COR_JANELA_BODY;
    uint8_t cor_opt2 = (item_selecionado == 2) ? COR_MENU_SEL : COR_JANELA_BODY;

    put_string_buffer(x + 2, y + 2, " 1. Terminal Shell          ", cor_opt0);
    put_string_buffer(x + 2, y + 4, " 2. Trocar Wallpaper (.img)  ", cor_opt1);
    put_string_buffer(x + 2, y + 6, " 3. Executar Programa BASIC ", cor_opt2);
}

void tui_desktop_start(void) {
    while (1) {
        tui_desenhar_desktop();
        tui_desenhar_menu();
        tui_desenhar_barra();
        tui_flush();

        char tecla = kbd_getchar();

        if (tecla == 'w' || tecla == 'W') {
            if (menu_aberto && item_selecionado > 0) item_selecionado--;
        }
        else if (tecla == 's' || tecla == 'S') {
            if (menu_aberto && item_selecionado < 2) item_selecionado++;
        }
        else if (tecla == 'm' || tecla == 'M' || tecla == ' ') {
            menu_aberto = !menu_aberto;
        }
        else if (tecla == '\n' || tecla == '\r') {
            if (menu_aberto) {
                if (item_selecionado == 0) {
                    // Fecha o menu antes de abrir o shell
                    menu_aberto = 0; 
                    
                    // Limpa a tela de verdade para o terminal respirar sozinho
                    vga_clear();
                    
                    // Roda o shell e TRAVA o desktop aqui até o usuário sair do shell
                    // (Certifique-se de chamar a função que roda o loop do seu shell, ex: shell_run ou shell_init)
                    shell_init(); 

                    // Quando o usuário der exit no shell, o fluxo volta pra cá e redesenha o desktop limpo
                }
                else if (item_selecionado == 1) {
                    wallpaper_index = (wallpaper_index + 1) % 3;
                }
                else if (item_selecionado == 2) {
                    menu_aberto = 0;
                    vga_clear();
                    basic_rodar_arquivo("teste.bas");
                    // Pausa opcional para ver o resultado do BASIC antes de voltar ao desktop
                }
            }
        }
    }
}

