// pic.c
#include "pic.h"
#include "idt.h" // Para uso de outb/inb

void pic_remap(void) {
    // Salva as máscaras atuais
    (void)inb(PIC1_DATA);
    (void)inb(PIC2_DATA);

    // Inicialização do Master e Slave em modo cascata
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Offset dos vetores de interrupção (IRQ0-7 -> 0x20-0x27 | IRQ8-15 -> 0x28-0x2F)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Configura a conexão em cascata
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Modo 8086 / x86
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restaura as máscaras travando tudo por padrão
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}

