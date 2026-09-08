#ifndef LEOFILES_H
#define LEOFILES_H

#include <stdint.h>

#define LEO_BLOCK_SIZE 512
#define LEO_BLOCK_FREE 0x0000
#define LEO_BLOCK_EOF  0xFFFF

struct LeoSuperblock {
    char magic[8];
    uint16_t total_blocks;
    uint16_t block_size;
    uint16_t root_block;
};

struct LeoDirEntry {
    char name[32];
    uint32_t size;
    uint16_t first_block;
    uint8_t is_directory;
    uint8_t used;
};

// Protótipos mantidos exatamente como antes
void leofiles_format(void);
int leofiles_create(const char *name, uint8_t is_dir);
int leofiles_write(const char *name, const char *data, uint32_t size);
int leofiles_read(const char *name, char *buffer, uint32_t size);
int leofiles_remove(const char *name);
void leofiles_list(void);

// Novas funções para suporte à árvore VFS e diretórios
int leofiles_create_in_dir(uint16_t dir_block, const char *name, uint8_t is_dir);
void leofiles_init_tree(void);
uint16_t leofiles_get_programas_block(void);

#endif

