#include "init.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "memory.h"
#include "paging.h"
#include "vga.h"
#include "ehci.h"
#include "mouse.h"
#include "elf_loader.h"

extern void isr33(void);
extern void isr44(void);

void kernel_init_all(void) {
    vga_clear();
    vga_puts("[+] Inicializando leonidas_OS...\n");

    gdt_init();
    vga_puts("[+] GDT configurada (Ring 0).\n");

    pic_remap();
    vga_puts("[+] PIC remapeado (IRQ0-15 -> 0x20-0x2F).\n");

    idt_init();
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E); // IRQ1 (Teclado)
    pic_unmask_irq(1);
    vga_puts("[+] IDT carregada e IRQ1 ativa.\n");

    memory_init();
    vga_puts("[+] Gerenciador de Memoria (Kmalloc) ativo.\n");

    __asm__ __volatile__("sti");
    vga_puts("[+] Interrupções da CPU habilitadas.\n\n");

    // Executa os módulos dinâmicos logo após a memória estar pronta
    modules_init();

    ehci_init(0xE0000000);

    pic_unmask_irq(12);
    mouse_init();

    paging_init();
}

