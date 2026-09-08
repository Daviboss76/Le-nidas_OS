extern void vga_puts(const char *str);

void module_init(void) {
    vga_puts("[MODULO DINAMICO] Modulo 'hello_mod' executado com sucesso!\n");
}

