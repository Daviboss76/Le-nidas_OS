// fs/leofiles_core.c
#include "leofiles.h"
#include "vga.h"

#define TOTAL_BLOCKS 2048

uint8_t virtual_disk[TOTAL_BLOCKS][LEO_BLOCK_SIZE];
uint16_t block_table[TOTAL_BLOCKS];

int fs_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

void fs_strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

uint16_t find_free_block(void) {
    for (uint16_t i = 10; i < TOTAL_BLOCKS; i++) {
        if (block_table[i] == LEO_BLOCK_FREE) return i;
    }
    return LEO_BLOCK_EOF;
}

void leofiles_format(void) {
    for (uint32_t i = 0; i < TOTAL_BLOCKS; i++) {
        block_table[i] = LEO_BLOCK_FREE;
        for (int j = 0; j < LEO_BLOCK_SIZE; j++) virtual_disk[i][j] = 0;
    }

    struct LeoSuperblock *sb = (struct LeoSuperblock*)virtual_disk[0];
    fs_strcpy(sb->magic, "LEOFILES");
    sb->total_blocks = TOTAL_BLOCKS;
    sb->block_size = LEO_BLOCK_SIZE;
    sb->root_block = 1;

    block_table[0] = LEO_BLOCK_EOF; // Superbloco
    block_table[1] = LEO_BLOCK_EOF; // Diretório Raiz (/)

    vga_puts("[+] LeoFiles formatado com sucesso!\n");
}

