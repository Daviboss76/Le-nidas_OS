// paging.c
#include "paging.h"
#include "vga.h"

// Page Directory e Primeira Page Table alinhados a 4096 bytes
static uint32_t page_directory[ENTRIES_PER_TABLE] __attribute__((aligned(4096)));
static uint32_t first_page_table[ENTRIES_PER_TABLE] __attribute__((aligned(4096)));

extern void load_page_directory(uint32_t *);
extern void enable_paging(void);

void paging_init(void) {
    vga_puts("[+] Configurando Paginacao de 4 KB (x86 Paging)...\n");

    // 1. Mapeia os primeiros 4 MB de memoria fisica (Identity Mapping)
    for (uint32_t i = 0; i < ENTRIES_PER_TABLE; i++) {
        // Endereco fisico = i * 4KB | Presente | Read/Write
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }

    // 2. Configura a primeira entrada do Page Directory apontando para a primeira Page Table
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;

    // 3. Marca as demais 1023 entradas do Page Directory como nao presentes
    for (uint32_t i = 1; i < ENTRIES_PER_TABLE; i++) {
        page_directory[i] = 0x00000002; // Read/Write, Not Present
    }

    // 4. Carrega o CR3 e ativa o bit PG no CR0
    load_page_directory(page_directory);
    enable_paging();

    vga_puts("[+] Paginacao de 4 KB ativa e funcional (CR0.PG = 1)!\n");
}

