// kernel/notepad.c
#include "vga.h"
#include "leofiles.h"

// Buffer global para armazenar o texto digitado no bloco de notas (ex: 4KB)
static char notepad_buffer[4096];
static int notepad_cursor = 0;

void notepad_limpar_buffer(void) {
    for (int i = 0; i < 4096; i++) {
        notepad_buffer[i] = '\0';
    }
    notepad_cursor = 0;
}

// Função principal do Bloco de Notas nativo
void notepad_abrir(const char *nome_arquivo) {
    vga_clear();
    vga_puts("=== BLOCO DE NOTAS - LE-NIDAS OS ===\n");
    vga_puts("Digite seu texto/codigo. Pressione ESC ou digite :wq para salvar e sair.\n");
    vga_puts("------------------------------------------------------------------\n");

    notepad_limpar_buffer();
    
    // Se o arquivo já existir, carrega o conteúdo atual para edição
    int lidos = leofiles_read(nome_arquivo, notepad_buffer, sizeof(notepad_buffer) - 1);
    if (lidos > 0) {
        notepad_cursor = lidos;
        notepad_buffer[notepad_cursor] = '\0';
        vga_puts(notepad_buffer); // Exibe o texto existente
    }

    // Loop de captura de texto (simulado com o buffer de entrada do shell/teclado)
    // Aqui você pode integrar com a função de leitura de linhas do seu teclado do kernel
    // ...
}

void notepad_salvar_arquivo(char *nome_arquivo) {
    // Grava tudo o que está no buffer de uma vez só no LeoFiles
    int res = leofiles_write(nome_arquivo, notepad_buffer, notepad_cursor);
    if (res == 0) {
        vga_puts("\n[+] Arquivo salvo com sucesso no LeoFiles!\n");
    } else {
        vga_puts("\n[-] Erro ao salvar o arquivo.\n");
    }
}

