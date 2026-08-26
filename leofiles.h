// leofiles.h
#ifndef LEOFILES_H
#define LEOFILES_H

#include <stdint.h>

#define LEO_BLOCK_SIZE      512
#define LEO_MAX_FILENAME    32
#define LEO_BLOCK_FREE      0x0000
#define LEO_BLOCK_EOF       0xFFFF

// Estrutura de uma Entrada de Arquivo/Diretório
struct LeoDirEntry {
    char     name[LEO_MAX_FILENAME]; // Nome do arquivo
    uint32_t size;                   // Tamanho em bytes
    uint16_t first_block;            // Primeiro bloco de dados
    uint8_t  is_directory;           // 1 se for pasta, 0 se for arquivo
    uint8_t  used;                   // 1 se a entrada estiver ocupada
} __attribute__((packed));

// Superbloco do LeoFiles
struct LeoSuperblock {
    char     magic[8];        // "LEOFILES"
    uint32_t total_blocks;   // Total de blocos no disco
    uint32_t block_size;     // Tamanho de cada bloco (512 bytes)
    uint16_t root_block;     // Bloco do diretório raiz
} __attribute__((packed));

void leofiles_format(void);
int  leofiles_create(const char *name, uint8_t is_dir);
int  leofiles_write(const char *name, const char *data, uint32_t size);
int  leofiles_read(const char *name, char *buffer, uint32_t size);
int  leofiles_remove(const char *name);
void leofiles_list(void);

#endif

