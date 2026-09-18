#include "vga.h"
#include "leofiles.h"

// Variáveis globais para a linguagem BASIC (V0 a V9)
static int var_memoria[10];

// Função auxiliar simples para converter inteiro em string e imprimir no VGA
void basic_print_int(int n) {
    if (n == 0) {
        vga_puts("0");
        return;
    }
    char buf[12];
    int i = 0;
    int neg = 0;
    if (n < 0) {
        neg = 1;
        n = -n;
    }
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    if (neg) {
        buf[i++] = '-';
    }
    buf[i] = '\0';

    // Inverte a string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        start++;
        end--;
    }
    vga_puts(buf);
}

void basic_executar_linha(char *linha) {
    while (*linha == ' ' || *linha == '\t') linha++;
    if (*linha == '\0' || *linha == '\n' || *linha == '\r' || *linha == '#') return;

    // Remove \r do final se houver
    int len = 0;
    while (linha[len] != '\0') len++;
    if (len > 0 && linha[len - 1] == '\r') linha[len - 1] = '\0';

    // Comando PRINT: pode imprimir texto entre aspas ou o valor de uma variável (ex: PRINT V1)
    if ((linha[0] == 'P' || linha[0] == 'p') && (linha[1] == 'R' || linha[1] == 'r')) {
        char *ptr = linha + 5;
        while (*ptr == ' ' || *ptr == '\t') ptr++;

        if ((ptr[0] == 'V' || ptr[0] == 'v') && ptr[1] >= '0' && ptr[1] <= '9') {
            int var_idx = ptr[1] - '0';
            basic_print_int(var_memoria[var_idx]);
            vga_puts("\n");
        } else {
            while (*ptr == '"') ptr++;
            char *fim = ptr;
            while (*fim != '\0' && *fim != '\r' && *fim != '\n') fim++;
            if (*(fim - 1) == '"') *(fim - 1) = '\0';
            vga_puts(ptr);
            vga_puts("\n");
        }
    }
    // Comando LEV V0 = 10 ou LEV V0 10 (Atribuição de variáveis)
    else if ((linha[0] == 'L' || linha[0] == 'l') && (linha[1] == 'E' || linha[1] == 'e')) {
        char *ptr = linha + 3;
        while (*ptr == ' ') ptr++;
        if (ptr[0] == 'V' || ptr[0] == 'v') {
            int var_idx = ptr[1] - '0';
            ptr += 2;
            while (*ptr == ' ' || *ptr == '=' || *ptr == '\t') ptr++;
            int valor = 0;
            int negativo = 0;
            if (*ptr == '-') {
                negativo = 1;
                ptr++;
            }
            while (*ptr >= '0' && *ptr <= '9') {
                valor = valor * 10 + (*ptr - '0');
                ptr++;
            }
            if (negativo) valor = -valor;
            if (var_idx >= 0 && var_idx < 10) {
                var_memoria[var_idx] = valor;
            }
        }
    }
    // Comando ADD V1 V2 (Soma V1 com V2 e guarda em V1)
    else if ((linha[0] == 'A' || linha[0] == 'a') && (linha[1] == 'D' || linha[1] == 'd') && (linha[2] == 'D' || linha[2] == 'd')) {
        char *ptr = linha + 3;
        while (*ptr == ' ') ptr++;
        if ((ptr[0] == 'V' || ptr[0] == 'v') && ptr[1] >= '0' && ptr[1] <= '9') {
            int dest_idx = ptr[1] - '0';
            ptr += 2;
            while (*ptr == ' ') ptr++;
            if ((ptr[0] == 'V' || ptr[0] == 'v') && ptr[1] >= '0' && ptr[1] <= '9') {
                int src_idx = ptr[1] - '0';
                if (dest_idx >= 0 && dest_idx < 10 && src_idx >= 0 && src_idx < 10) {
                    var_memoria[dest_idx] += var_memoria[src_idx];
                }
            }
        }
    }
    // Comando SUB V1 V2 (Subtrai V2 de V1 e guarda em V1)
    else if ((linha[0] == 'S' || linha[0] == 's') && (linha[1] == 'U' || linha[1] == 'u') && (linha[2] == 'B' || linha[2] == 'b')) {
        char *ptr = linha + 3;
        while (*ptr == ' ') ptr++;
        if ((ptr[0] == 'V' || ptr[0] == 'v') && ptr[1] >= '0' && ptr[1] <= '9') {
            int dest_idx = ptr[1] - '0';
            ptr += 2;
            while (*ptr == ' ') ptr++;
            if ((ptr[0] == 'V' || ptr[0] == 'v') && ptr[1] >= '0' && ptr[1] <= '9') {
                int src_idx = ptr[1] - '0';
                if (dest_idx >= 0 && dest_idx < 10 && src_idx >= 0 && src_idx < 10) {
                    var_memoria[dest_idx] -= var_memoria[src_idx];
                }
            }
        }
    }
    // Comando BEEP (Emite um bipe simples no speaker do PC se suportado)
    else if ((linha[0] == 'B' || linha[0] == 'b') && (linha[1] == 'E' || linha[1] == 'e') && (linha[2] == 'E' || linha[2] == 'p')) {
        vga_puts("\a"); // Caractere de alerta do terminal
    }
    // Comando INFO (Exibe o estado de todas as variáveis V0 a V9)
    else if ((linha[0] == 'I' || linha[0] == 'i') && (linha[1] == 'N' || linha[1] == 'n') && (linha[2] == 'F' || linha[2] == 'f')) {
        vga_puts("--- REGISTRADORES BASIC --- \n");
        for (int i = 0; i < 10; i++) {
            vga_puts("V");
            char idx_str[2] = { '0' + i, '\0' };
            vga_puts(idx_str);
            vga_puts(" = ");
            basic_print_int(var_memoria[i]);
            vga_puts("\n");
        }
    }
    // Comando SYSTEM <comando>
    else if ((linha[0] == 'S' || linha[0] == 's') && (linha[1] == 'Y' || linha[1] == 'y')) {
        char *ptr = linha + 6;
        while (*ptr == ' ') ptr++;
        vga_puts("[BASIC SYSTEM] Executando: ");
        vga_puts(ptr);
        vga_puts("\n");
    }
    else {
        vga_puts("[-] Erro de sintaxe na linguagem BASIC.\n");
    }
}

void basic_rodar_arquivo(char *nome_arquivo) {
    char buffer[4096];
    int lidos = leofiles_read(nome_arquivo, buffer, sizeof(buffer) - 1);

    if (lidos < 0) {
        vga_puts("[-] Erro: Script nao encontrado no LeoFiles.\n");
        return;
    }
    buffer[lidos] = '\0';

    char *linha_atual = buffer;
    for (int i = 0; i < lidos; i++) {
        if (buffer[i] == '\n') {
            buffer[i] = '\0';
            basic_executar_linha(linha_atual);
            linha_atual = &buffer[i + 1];
        }
    }
    if (*linha_atual != '\0') {
        basic_executar_linha(linha_atual);
    }
}

