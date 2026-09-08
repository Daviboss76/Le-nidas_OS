#include <stdint.h>
#include "elf_loader.h"
#include "memory.h"
#include "vga.h"
#include "hello_mod.h"

typedef struct {
    const char *name;
    uint32_t addr;
} kernel_symbol_t;

extern void vga_puts(const char *str);

static kernel_symbol_t kernel_symbols[] = {
    {"vga_puts", (uint32_t)vga_puts},
    {"kmalloc",  (uint32_t)kmalloc},
    {0, 0}
};

static uint32_t find_kernel_symbol(const char *name) {
    for (int i = 0; kernel_symbols[i].name != 0; i++) {
        const char *s1 = name;
        const char *s2 = kernel_symbols[i].name;
        while (*s1 && (*s1 == *s2)) { s1++; s2++; }
        if (*s1 == *s2) return kernel_symbols[i].addr;
    }
    return 0;
}

int load_module(uint8_t *module_data) {
    elf32_header_t *hdr = (elf32_header_t *)module_data;

    if (*(uint32_t *)hdr->e_ident != ELF_MAGIC) {
        vga_puts("[-] Erro: Arquivo nao e um ELF valido.\n");
        return -1;
    }

    elf32_shdr_t *sections = (elf32_shdr_t *)(module_data + hdr->e_shoff);

    // 1. Aloca memória para as seções (.text, .data, .bss)
    for (int i = 0; i < hdr->e_shnum; i++) {
        if (sections[i].sh_flags & 0x2) { // SHF_ALLOC
            uint32_t mem = (uint32_t)kmalloc(sections[i].sh_size);
            if (sections[i].sh_type != 8) { // SHT_NOBITS (.bss)
                uint8_t *src = module_data + sections[i].sh_offset;
                uint8_t *dest = (uint8_t *)mem;
                for (uint32_t b = 0; b < sections[i].sh_size; b++) dest[b] = src[b];
            }
            sections[i].sh_addr = mem;
        }
    }

    // 2. Resolve as realocações dinâmicas (Relocations)
    for (int i = 0; i < hdr->e_shnum; i++) {
        if (sections[i].sh_type == 9) { // SHT_REL
            elf32_rel_t *rels = (elf32_rel_t *)(module_data + sections[i].sh_offset);
            int rel_count = sections[i].sh_size / sizeof(elf32_rel_t);

            elf32_shdr_t *target_sec = &sections[sections[i].sh_info];
            elf32_shdr_t *symtab_sec = &sections[sections[i].sh_link];
            elf32_sym_t *symtab = (elf32_sym_t *)(module_data + symtab_sec->sh_offset);
            const char *strtab = (char *)(module_data + sections[symtab_sec->sh_link].sh_offset);

            for (int r = 0; r < rel_count; r++) {
                uint32_t sym_idx = ELF32_R_SYM(rels[r].r_info);
                uint8_t type = ELF32_R_TYPE(rels[r].r_info);

                elf32_sym_t *sym = &symtab[sym_idx];
                uint32_t sym_val = 0;

                if (sym->st_shndx == 0) { // Símbolo externo do kernel
                    const char *sym_name = strtab + sym->st_name;
                    sym_val = find_kernel_symbol(sym_name);
                    if (!sym_val) {
                        vga_puts("[-] Erro: Simbolo nao resolvido: ");
                        vga_puts(sym_name);
                        vga_puts("\n");
                        return -2;
                    }
                } else {
                    sym_val = sections[sym->st_shndx].sh_addr + sym->st_value;
                }

                uint32_t *patch_addr = (uint32_t *)(target_sec->sh_addr + rels[r].r_offset);

                if (type == R_386_32) {
                    *patch_addr += sym_val;
                } else if (type == R_386_PC32) {
                    *patch_addr += sym_val - (uint32_t)patch_addr;
                }
            }
        }
    }

    // 3. Executa a função module_init() do módulo
    for (int i = 0; i < hdr->e_shnum; i++) {
        if (sections[i].sh_type == 2) { // SHT_SYMTAB
            elf32_sym_t *symtab = (elf32_sym_t *)(module_data + sections[i].sh_offset);
            const char *strtab = (char *)(module_data + sections[sections[i].sh_link].sh_offset);
            int count = sections[i].sh_size / sizeof(elf32_sym_t);

            for (int s = 0; s < count; s++) {
                const char *name = strtab + symtab[s].st_name;
                if (name[0] == 'm' && name[1] == 'o' && name[2] == 'd' && name[7] == 'i') {
                    uint32_t init_fn = sections[symtab[s].st_shndx].sh_addr + symtab[s].st_value;
                    void (*entry)(void) = (void (*)(void))init_fn;
                    vga_puts("[LKM] Carregando modulo dinamicamente...\n");
                    entry();
                    return 0;
                }
            }
        }
    }

    return 0;
}

void modules_init(void) {
    vga_puts("[+] Carregador de modulos dinâmicos ativo.\n");
    load_module((uint8_t *)hello_mod_o);
}

