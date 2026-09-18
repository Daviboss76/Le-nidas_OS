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
#include "RTC.h"
#include "AC97.h"
#include "splash.h"
#include "leofiles.h"
#include "rtl8139.h"

// Declarações externas
extern void isr33(void);
extern void isr44(void);
extern void leofiles_format(void);
extern void cpu_enable_interrupts(void);
extern void modules_init(void);

void kernel_init_all(void) {
    vga_clear();
    vga_puts("[+] Inicializando Le-nidas OS...\n");

    gdt_init();
    vga_puts("[+] GDT configurada.\n");

    pic_remap();
    vga_puts("[+] PIC remapeado.\n");

    idt_init();
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);
    pic_unmask_irq(1);
    vga_puts("[+] IDT carregada.\n");

    memory_init();
    vga_puts("[+] Gerenciador de Memoria OK.\n");

    // Inicializa e formata o sistema de arquivos LeoFiles
    leofiles_format();

    cpu_enable_interrupts();
    vga_puts("[+] Interrupcoes ativadas.\n");

    // Chama o inicializador de módulos
    modules_init();

    ehci_init(0xE0000000);
    pic_unmask_irq(12);
    mouse_init();

    paging_init();
    vga_puts("[+] Paginacao habilitada.\n");

    vga_puts("[+] RTC (Real Time Clock) inicializado.\n");

    rtc_time_t now;
    char time_buf[20];
    rtc_read_datetime(&now);
    rtc_format_datetime(&now, time_buf);

    vga_puts("[+] Data/Hora de boot: ");
    vga_puts(time_buf);
    vga_puts("\n");

    vga_puts("[+] Detectando placa de som...\n");
    if (ac97_init()) {
        vga_puts("[+] AC'97 Audio inicializado com sucesso!\n");
        ac97_play_tone(440, 200);
    } else {
        vga_puts("[-] Dispositivo AC'97 nao encontrado.\n");
    }

        vga_puts("[-] inicializado rede rtl8139.\n");
        rtl8139_init();
}

