// paging.h
#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define ENTRIES_PER_TABLE 1024

// Atributos dos bits de cada entrada (PDE / PTE)
#define PAGE_PRESENT  0x1 // Bit 0: Página presente na memória
#define PAGE_RW       0x2 // Bit 1: Leitura/Escrita (1 = R/W, 0 = Read-Only)
#define PAGE_USER     0x4 // Bit 2: Usuário/Kernel (1 = User, 0 = Supervisor/Kernel)

void paging_init(void);

#endif

