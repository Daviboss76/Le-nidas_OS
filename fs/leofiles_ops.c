// fs/leofiles_ops.c
#include "leofiles.h"
#include "vga.h"

extern uint8_t virtual_disk[2048][LEO_BLOCK_SIZE];
extern uint16_t block_table[2048];
extern int fs_strcmp(const char *a, const char *b);
extern void fs_strcpy(char *dest, const char *src);
extern uint16_t find_free_block(void);

// Cria um arquivo/pasta dentro de um bloco de diretório específico
int leofiles_create_in_dir(uint16_t dir_block, const char *name, uint8_t is_dir) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[dir_block];

    for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (!entries[i].used) {
            fs_strcpy(entries[i].name, name);
            entries[i].size = 0;
            entries[i].first_block = find_free_block();
            entries[i].is_directory = is_dir;
            entries[i].used = 1;

            if (entries[i].first_block != LEO_BLOCK_EOF) {
                block_table[entries[i].first_block] = LEO_BLOCK_EOF;
            }
            return 0;
        }
    }
    return -1;
}

int leofiles_create(const char *name, uint8_t is_dir) {
    return leofiles_create_in_dir(1, name, is_dir);
}

int leofiles_write(const char *name, const char *data, uint32_t size) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];

    for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && fs_strcmp(entries[i].name, name) == 0) {
            uint16_t curr = entries[i].first_block;
            uint32_t written = 0;

            while (written < size) {
                uint32_t chunk = (size - written > LEO_BLOCK_SIZE) ? LEO_BLOCK_SIZE : (size - written);
                for (uint32_t b = 0; b < chunk; b++) virtual_disk[curr][b] = data[written + b];
                written += chunk;
                entries[i].size = written;

                if (written < size) {
                    uint16_t next = find_free_block();
                    if (next == LEO_BLOCK_EOF) return -1;
                    block_table[curr] = next;
                    curr = next;
                    block_table[curr] = LEO_BLOCK_EOF;
                }
            }
            return 0;
        }
    }
    return -1;
}

int leofiles_read(const char *name, char *buffer, uint32_t size) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];

    for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && fs_strcmp(entries[i].name, name) == 0) {
            uint16_t curr = entries[i].first_block;
            uint32_t read_bytes = 0;

            while (curr != LEO_BLOCK_EOF && read_bytes < size && read_bytes < entries[i].size) {
                uint32_t chunk = (entries[i].size - read_bytes > LEO_BLOCK_SIZE) ? LEO_BLOCK_SIZE : (entries[i].size - read_bytes);
                for (uint32_t b = 0; b < chunk; b++) buffer[read_bytes + b] = virtual_disk[curr][b];
                read_bytes += chunk;
                curr = block_table[curr];
            }
            buffer[read_bytes] = '\0';
            return read_bytes;
        }
    }
    return -1;
}

int leofiles_remove(const char *name) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];

    for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used && fs_strcmp(entries[i].name, name) == 0) {
            uint16_t curr = entries[i].first_block;
            while (curr != LEO_BLOCK_EOF) {
                uint16_t next = block_table[curr];
                block_table[curr] = LEO_BLOCK_FREE;
                curr = next;
            }
            entries[i].used = 0;
            return 0;
        }
    }
    return -1;
}

void leofiles_list(void) {
    struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[1];
    vga_puts("NOME\t\tTIPO\tTAMANHO\n");
    vga_puts("-----------------------------------\n");

    for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
        if (entries[i].used) {
            vga_puts(entries[i].name);
            vga_puts(entries[i].is_directory ? "\t[DIR]\n" : "\t[ARQ]\n");
        }
    }
}

