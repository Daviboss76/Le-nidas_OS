// leofiles.c
#include "leofiles.h"
#include "vga.h"
#include "memory.h"

// Memória simulada do disco virtual de 1MB (2048 blocos de 512 bytes)
#define TOTAL_BLOCKS 2048
static uint8_t virtual_disk[TOTAL_BLOCKS][LEO_BLOCK_SIZE];
static uint16_t block_table[TOTAL_BLOCKS]; // Tabela de Blocos (FAT)

static int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

static void strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

// Procura por um bloco livre na tabela
static uint16_t find_free_block(void) {
    for (uint16_t i = 10; i < TOTAL_BLOCKS; i++) {
        if (block_table[i] == LEO_BLOCK_FREE) return i;
    }
    return LEO_BLOCK_EOF;
}

// Formatação do LeoFiles
void leofiles_format(void) {
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        block_table[i] = LEO_BLOCK_FREE;
        for (int j = 0; j < LEO_BLOCK_SIZE; j++) virtual_disk[i][j] = 0;
    }

    struct LeoSuperblock *sb = (struct LeoSuperblock*)virtual_disk[0];
    strcpy(sb->magic, "LEOFILES");
    sb->total_blocks = TOTAL_BLOCKS;
    sb->block_size = LEO_BLOCK_SIZE;
    sb->root_block = 1;

    block_table[0] = LEO_BLOCK_EOF; // Superbloco
    block_table[1] = LEO_BLOCK_EOF; // Diretório Raiz
    
    vga_puts("[+] Sistema LeoFiles formatado com sucesso!\n");
}

// Criar arquivo ou pasta
int leofiles_create(const char *name, uint8_t is_dir) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];
    
    for (int i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (!entries[i].used) {
            strcpy(entries[i].name, name);
            entries[i].size = 0;
            entries[i].first_block = find_free_block();
            entries[i].is_directory = is_dir;
            entries[i].used = 1;

            if (entries[i].first_block != LEO_BLOCK_EOF) {
                block_table[entries[i].first_block] = LEO_BLOCK_EOF;
            }
            return 0; // Sucesso
        }
    }
    return -1; // Diretório Cheio
}

// Escrever dados em um arquivo
int leofiles_write(const char *name, const char *data, uint32_t size) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];

    for (int i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && strcmp(entries[i].name, name) == 0) {
            uint16_t curr_block = entries[i].first_block;
            uint32_t written = 0;

            while (written < size) {
                uint32_t chunk = (size - written > LEO_BLOCK_SIZE) ? LEO_BLOCK_SIZE : (size - written);
                for (uint32_t b = 0; b < chunk; b++) {
                    virtual_disk[curr_block][b] = data[written + b];
                }
                written += chunk;
                entries[i].size = written;

                if (written < size) {
                    uint16_t next = find_free_block();
                    if (next == LEO_BLOCK_EOF) return -1; // Disco cheio
                    block_table[curr_block] = next;
                    curr_block = next;
                    block_table[curr_block] = LEO_BLOCK_EOF;
                }
            }
            return 0;
        }
    }
    return -1; // Arquivo não encontrado
}

// Ler dados de um arquivo
int leofiles_read(const char *name, char *buffer, uint32_t size) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];

    for (int i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && strcmp(entries[i].name, name) == 0) {
            uint16_t curr_block = entries[i].first_block;
            uint32_t read_bytes = 0;

            while (curr_block != LEO_BLOCK_EOF && read_bytes < size && read_bytes < entries[i].size) {
                uint32_t chunk = (entries[i].size - read_bytes > LEO_BLOCK_SIZE) ? LEO_BLOCK_SIZE : (entries[i].size - read_bytes);
                for (uint32_t b = 0; b < chunk; b++) {
                    buffer[read_bytes + b] = virtual_disk[curr_block][b];
                }
                read_bytes += chunk;
                curr_block = block_table[curr_block];
            }
            buffer[read_bytes] = '\0';
            return read_bytes;
        }
    }
    return -1;
}

// Remover arquivo
int leofiles_remove(const char *name) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];

    for (int i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && strcmp(entries[i].name, name) == 0) {
            uint16_t curr_block = entries[i].first_block;
            
            // Libera a cadeia de blocos no FAT
            while (curr_block != LEO_BLOCK_EOF) {
                uint16_t next = block_table[curr_block];
                block_table[curr_block] = LEO_BLOCK_FREE;
                curr_block = next;
            }

            entries[i].used = 0;
            return 0;
        }
    }
    return -1;
}

// Listar arquivos no diretório
void leofiles_list(void) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];
    vga_puts("NOME\t\tTIPO\tTAMANHO\n");
    vga_puts("-----------------------------------\n");

    for (int i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used) {
            vga_puts(entries[i].name);
            vga_puts(entries[i].is_directory ? "\t[DIR]\t" : "\t[ARQ]\t");
            
            // Impressão simples de tamanho
            if (entries[i].size == 0) vga_puts("0B\n");
            else vga_puts(">0B\n");
        }
    }
}

