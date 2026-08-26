// mouse.c
#include "mouse.h"
#include "idt.h"
#include "pic.h"
#include "vga.h"

static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
static int32_t mouse_x = 40; // Posição inicial no centro da tela VGA (80x25)
static int32_t mouse_y = 12;

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

static void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4); // Informa ao controlador que o dado vai para o mouse
    mouse_wait(1);
    outb(0x60, write);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_handler(void) {
    uint8_t status = inb(0x64);
    if (!(status & 0x01)) {
        pic_send_eoi(12);
        return;
    }

    uint8_t data = inb(0x60);

    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = data;
            // Valida se o primeiro byte tem o bit 3 ajustado (sincronização do pacote PS/2)
            if (data & 0x08) mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;

            // Calcula o movimento relativo
            int8_t rel_x = mouse_byte[1];
            int8_t rel_y = mouse_byte[2];

            mouse_x += rel_x / 8; // Sensibilidade reduzida
            mouse_y -= rel_y / 8;

            // Mantém o cursor dentro dos limites da tela (80x25)
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= 80) mouse_x = 79;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= 25) mouse_y = 24;

            break;
    }

    pic_send_eoi(12);
}

void mouse_init(void) {
    uint8_t status;

    // Habilita a segunda porta PS/2
    mouse_wait(1);
    outb(0x64, 0xA8);

    // Habilita as interrupções do mouse no byte de configuração do 8042
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2); // Ativa o bit 1 (IRQ12 enable)
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    // Ativa o envio de pacotes do mouse
    mouse_write(0xF4);
    mouse_read(); // Descarta o ACK (0xFA)

    // Desbloqueia a IRQ12 no PIC Slave
    pic_unmask_irq(12);

    vga_puts("[+] Driver de Mouse PS/2 (IRQ12) inicializado!\n");
}

