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

// Inicializa as pastas padrão do sistema de forma segura (verifica se já existem)
void leofiles_init_tree(void) {
    // 1. Garantir/Localizar /system, /boot, /root na Raiz (bloco 1)
    sys_dir_block = find_dir_block(1, "system");
    if (sys_dir_block == 0) {
        leofiles_create_in_dir(1, "system", 1);
        sys_dir_block = find_dir_block(1, "system");
    }

    if (find_dir_block(1, "boot") == 0) {
        leofiles_create_in_dir(1, "boot", 1);
    }
    if (find_dir_block(1, "root") == 0) {
        leofiles_create_in_dir(1, "root", 1);
    }

    // 2. Garantir/Localizar /system/usr
    if (sys_dir_block != 0) {
        usr_dir_block = find_dir_block(sys_dir_block, "usr");
        if (usr_dir_block == 0) {
            leofiles_create_in_dir(sys_dir_block, "usr", 1);
            usr_dir_block = find_dir_block(sys_dir_block, "usr");
        }
    }

    // 3. Garantir/Localizar /system/usr/programas
    if (usr_dir_block != 0) {
        prog_dir_block = find_dir_block(usr_dir_block, "programas");
        if (prog_dir_block == 0) {
            leofiles_create_in_dir(usr_dir_block, "programas", 1);
            prog_dir_block = find_dir_block(usr_dir_block, "programas");
        }
    }

    vga_puts("[+] Arvore /system /boot /usr /programas /root verificada e montada com sucesso!\n");
}

uint16_t leofiles_get_programas_block(void) {
    // Caso a variável ainda esteja zerada, tenta buscar dinamicamente na árvore
    if (prog_dir_block == 0) {
        uint16_t s = find_dir_block(1, "system");
        uint16_t u = s ? find_dir_block(s, "usr") : 0;
        prog_dir_block = u ? find_dir_block(u, "programas") : 0;
    }
    return prog_dir_block;
}

