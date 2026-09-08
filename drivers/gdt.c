// gdt.c
#include "gdt.h"

struct GDTEntry gdt[3];
struct GDTPtr   gdtp;

extern void gdt_flush(uint32_t gdt_ptr_addr);

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void gdt_init(void) {
    gdtp.limit = (sizeof(struct GDTEntry) * 3) - 1;
    gdtp.base  = (uint32_t)&gdt;

    // 1. Descriptor Nulo
    gdt_set_gate(0, 0, 0, 0, 0);

    // 2. Kernel Code Segment (0x08): Ring 0, Executável, Leitura
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 3. Kernel Data Segment (0x10): Ring 0, Leitura/Escrita
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Carrega a GDT e atualiza os registradores de segmento (CS, DS, SS, etc)
    gdt_flush((uint32_t)&gdtp);
}

