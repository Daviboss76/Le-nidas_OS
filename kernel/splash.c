#include "splash.h"

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Alterna o controlador VGA para o Modo 13h (320x200, 256 cores) via registradores I/O
static void set_vga_mode_13h(void) {
    // Sequência de registradores para ativar 320x200x256
    outb(0x3C2, 0x63);
    
    uint8_t seq[] = { 0x03, 0x01, 0x0F, 0x00, 0x0E };
    for (uint8_t i = 0; i < 5; i++) {
        outb(0x3C4, i);
        outb(0x3C5, seq[i]);
    }

    outb(0x3D4, 0x11);
    outb(0x3D5, 0x00);

    uint8_t crtc[] = { 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
                       0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF };
    for (uint8_t i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc[i]);
    }

    uint8_t gc[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF };
    for (uint8_t i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, gc[i]);
    }
}

void splash_clear(uint8_t color) {
    uint8_t *vga = VGA_GFX_MEM;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        vga[i] = color;
    }
}

void splash_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VGA_GFX_MEM[y * SCREEN_WIDTH + x] = color;
    }
}

// Desenha um retângulo preenchido (usado para a barra de progresso)
static void draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            splash_draw_pixel(i, j, color);
        }
    }
}

void splash_show(void) {
    // 1. Entra no Modo Gráfico
    set_vga_mode_13h();

    // 2. Fundo Azul Escuro (Cor 0x01 na paleta VGA)
    splash_clear(0x01);

    // 3. Moldura de destaque central (Caixa da Logomarca)
    draw_rect(60, 40, 200, 60, 0x0F); // Retângulo Branco
    draw_rect(62, 42, 196, 56, 0x00); // Interior Preto

    // 4. Borda e Moldura da Barra de Progresso
    draw_rect(59, 139, 202, 17, 0x0F); // Borda Externa Branca
    draw_rect(60, 140, 200, 15, 0x08); // Fundo Cinza Escuro

    // 5. Animação de carregamento (Barra de Progresso)
    for (int progress = 0; progress <= 196; progress += 4) {
        // Preenche a barra com a cor Verde Azulado (0x0A)
        draw_rect(62, 142, progress, 11, 0x0A);

        // Delay simples para visualizar o carregamento
        for (volatile int d = 0; d < 3000000; d++);
    }

    // Delay final exibindo a tela concluída
    for (volatile int d = 0; d < 10000000; d++);
}

