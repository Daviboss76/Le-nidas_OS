// fs/leofiles_vfs.c
#include "leofiles.h"
#include "vga.h"

extern uint8_t virtual_disk[2048][LEO_BLOCK_SIZE];
extern int fs_strcmp(const char *a, const char *b);
extern int leofiles_create_in_dir(uint16_t dir_block, const char *name, uint8_t is_dir);

static uint16_t sys_dir_block = 0;
static uint16_t usr_dir_block = 0;
static uint16_t prog_dir_block = 0;

// Busca o bloco de um subdiretório pelo nome a partir do bloco pai
static uint16_t find_dir_block(uint16_t parent_block, const char *name) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[parent_block];
    for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && entries[i].is_directory && fs_strcmp(entries[i].name, name) == 0) {
            return entries[i].first_block;
        }
    }
    return 0;
}

// Inicializa as pastas padrão do sistema
void leofiles_init_tree(void) {
    // 1. Criar pastas no diretório Raiz (bloco 1)
    leofiles_create_in_dir(1, "system", 1);
    leofiles_create_in_dir(1, "boot", 1);
    leofiles_create_in_dir(1, "root", 1);

    sys_dir_block = find_dir_block(1, "system");

    // 2. Criar /system/usr
    if (sys_dir_block != 0) {
        leofiles_create_in_dir(sys_dir_block, "usr", 1);
        usr_dir_block = find_dir_block(sys_dir_block, "usr");
    }

    // 3. Criar /system/usr/programas
    if (usr_dir_block != 0) {
        leofiles_create_in_dir(usr_dir_block, "programas", 1);
        prog_dir_block = find_dir_block(usr_dir_block, "programas");
    }

    vga_puts("[+] Arvore /system /boot /usr /programas /root montada com sucesso!\n");
}

uint16_t leofiles_get_programas_block(void) {
    return prog_dir_block;
}

