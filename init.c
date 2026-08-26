// init.c
#include "init.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "memory.h"
#include "vga.h"
#include "ehci.h"
#include "mouse.h"

extern void isr33(void);
extern void isr44(void);

void kernel_init_all(void) {
    // 1. Limpa o display VGA
    vga_clear();
    vga_puts("[+] Inicializando leonidas_OS...\n");

    // 2. Configura os segmentos de memória
    gdt_init();
    vga_puts("[+] GDT configurada (Ring 0).\n");

    // 3. Remapeia o PIC 8259A
    pic_remap();
    vga_puts("[+] PIC remapeado (IRQ0-15 -> 0x20-0x2F).\n");

    // 4. Configura a Tabela de Interrupções
    idt_init();
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E); // IRQ1 (Teclado)
    pic_unmask_irq(1);                             // Desbloqueia Teclado
    vga_puts("[+] IDT carregada e IRQ1 ativa.\n");

    // 5. Inicializa o Heap / Kmalloc
    memory_init();
    vga_puts("[+] Gerenciador de Memoria (Kmalloc) ativo.\n");

    // 6. Habilita interrupções globais da CPU (STI)
    __asm__ __volatile__("sti");
    vga_puts("[+] Interrupções da CPU habilitadas.\n\n");

    // Chama o EHCI passando o endereço base MMIO simulado
    ehci_init(0xE0000000); 

    // chama o mouse
    pic_unmask_irq(12);
    mouse_init();
}


