// kernel/tui_desktop.c
#include "tui_desktop.h"
#include "leofiles.h"
#include "vga.h"

#define VGA_MEM ((volatile uint16_t*)0xB8000)
#define COLS 80
#define ROWS 25

extern uint8_t virtual_disk[2048][LEO_BLOCK_SIZE];

// Função auxiliar para combinar o caractere ASCII e a cor VGA
static inline uint16_t vga_entry(unsigned char ch, uint8_t color) {
    return (uint16_t)ch | ((uint16_t)color << 8);
}

void tui_desktop_start(void) {
    // Atributos de Cores VGA (Fundo + Texto)
    uint8_t bg_white   = 0xF0; // Papel de parede branco, texto preto
    uint8_t win_bg     = 0x70; // Janela cinza claro
    uint8_t title_bg   = 0x8F; // Barra de título cinza escuro
    uint8_t btn_close  = 0x4F; // Botão fechar vermelho
    uint8_t btn_min    = 0x8F; // Botão minimizar cinza
    
    // Cores da Barra de Tarefas
    uint8_t bar_yellow = 0xE0; // Bloco amarelo
    uint8_t bar_btn    = 0xA0; // Botão LEONIDAS verde/amarelo
    uint8_t bar_blue   = 0x10; // Bloco azul central
    uint8_t bar_green  = 0x2F; // Bloco verde (relógio)

    // 1. Papel de parede em branco
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            VGA_MEM[y * COLS + x] = vga_entry(' ', bg_white);
        }
    }

    // 2. Janela Quadrada dos Programas (Posição: X=5, Y=3, L=30, A=14)
    int wx = 5, wy = 3, ww = 30, wh = 14;

    // Fundo da Janela
    for (int i = 0; i < wh; i++) {
        for (int j = 0; j < ww; j++) {
            VGA_MEM[(wy + i) * COLS + (wx + j)] = vga_entry(' ', win_bg);
        }
    }

    // Bordas da Janela
    for (int i = 0; i < ww; i++) {
        VGA_MEM[wy * COLS + (wx + i)] = vga_entry(196, win_bg);
        VGA_MEM[(wy + wh - 1) * COLS + (wx + i)] = vga_entry(196, win_bg);
    }
    for (int i = 0; i < wh; i++) {
        VGA_MEM[(wy + i) * COLS + wx] = vga_entry(179, win_bg);
        VGA_MEM[(wy + i) * COLS + (wx + ww - 1)] = vga_entry(179, win_bg);
    }
    VGA_MEM[wy * COLS + wx] = vga_entry(218, win_bg);
    VGA_MEM[wy * COLS + (wx + ww - 1)] = vga_entry(191, win_bg);
    VGA_MEM[(wy + wh - 1) * COLS + wx] = vga_entry(192, win_bg);
    VGA_MEM[(wy + wh - 1) * COLS + (wx + ww - 1)] = vga_entry(217, win_bg);

    // Barra de Título
    for (int j = 1; j < ww - 1; j++) {
        VGA_MEM[(wy + 1) * COLS + (wx + j)] = vga_entry(' ', title_bg);
    }

    // Texto da Barra de Título
    const char *title = "Programas";
    for (int i = 0; title[i]; i++) {
        VGA_MEM[(wy + 1) * COLS + (wx + 2 + i)] = vga_entry(title[i], title_bg);
    }

    // Botões Minimizar [-] e Fechar [X]
    VGA_MEM[(wy + 1) * COLS + (wx + ww - 6)] = vga_entry('[', title_bg);
    VGA_MEM[(wy + 1) * COLS + (wx + ww - 5)] = vga_entry('-', btn_min);
    VGA_MEM[(wy + 1) * COLS + (wx + ww - 4)] = vga_entry(']', title_bg);
    VGA_MEM[(wy + 1) * COLS + (wx + ww - 3)] = vga_entry('[', title_bg);
    VGA_MEM[(wy + 1) * COLS + (wx + ww - 2)] = vga_entry('X', btn_close);

    // 3. Listagem dinâmica de arquivos de /system/usr/programas
    uint16_t prog_block = leofiles_get_programas_block();
    int line_y = wy + 3;

    if (prog_block != 0) {
        struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[prog_block];
        for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
            if (entries[i].used) {
                // Desenha ícone de programa e o nome
                VGA_MEM[line_y * COLS + (wx + 2)] = vga_entry(16, win_bg); // Ícone '>'
                for (int c = 0; entries[i].name[c] != '\0'; c++) {
                    VGA_MEM[line_y * COLS + (wx + 4 + c)] = vga_entry(entries[i].name[c], win_bg);
                }
                line_y += 2;
                if (line_y >= wy + wh - 1) break;
            }
        }
    }

    // 4. Barra de Tarefas Inferior (Linhas 22 e 23)
    // Seção Amarela (Esquerda - Botão LEONIDAS)
    for (int x = 2; x < 25; x++) {
        VGA_MEM[22 * COLS + x] = vga_entry(' ', bar_yellow);
        VGA_MEM[23 * COLS + x] = vga_entry(' ', bar_yellow);
    }
    // Cantos arredondados na barra de tarefas (caracteres ASCII estendidos)
    VGA_MEM[22 * COLS + 2] = vga_entry('(', bar_yellow);
    VGA_MEM[23 * COLS + 2] = vga_entry('(', bar_yellow);

    // Botão "LEONIDAS"
    const char *btn_txt = " LEONIDAS ";
    for (int i = 0; btn_txt[i]; i++) {
        VGA_MEM[22 * COLS + (4 + i)] = vga_entry(btn_txt[i], bar_btn);
        VGA_MEM[23 * COLS + (4 + i)] = vga_entry(' ', bar_btn);
    }

    // Seção Azul (Meio)
    for (int x = 25; x < 50; x++) {
        VGA_MEM[22 * COLS + x] = vga_entry(' ', bar_blue);
        VGA_MEM[23 * COLS + x] = vga_entry(' ', bar_blue);
    }

    // Seção Verde (Direita - Relógio e Data)
    for (int x = 50; x < 78; x++) {
        VGA_MEM[22 * COLS + x] = vga_entry(' ', bar_green);
        VGA_MEM[23 * COLS + x] = vga_entry(' ', bar_green);
    }
    VGA_MEM[22 * COLS + 77] = vga_entry(')', bar_green);
    VGA_MEM[23 * COLS + 77] = vga_entry(')', bar_green);

    // Texto de Hora e Data
    const char *time_str = "HORA 00:00";
    const char *date_str = "30/08/26";

    for (int i = 0; time_str[i]; i++) {
        VGA_MEM[22 * COLS + (55 + i)] = vga_entry(time_str[i], bar_green);
    }
    for (int i = 0; date_str[i]; i++) {
        VGA_MEM[23 * COLS + (56 + i)] = vga_entry(date_str[i], bar_green);
    }
}

