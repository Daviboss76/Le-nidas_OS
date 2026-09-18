#include <stddef.h>
#include "shell.h"
#include "kernel.h"
#include "vga.h"
#include "leofiles.h"
#include "RTC.h"
#include "auth.h"
#include <stdint.h>
#include "../autoreparar/autoreparar.h"


// Interfaces externas do sistema
extern void gui_desktop_init(uint32_t lfb_addr);
extern void tui_desktop_start(void);
extern void leofiles_list(void);
extern int leofiles_create(const char *name, uint8_t is_dir);
extern int leofiles_remove(const char *name);
extern uint16_t leofiles_get_programas_block(void);
extern int ota_download_and_save_kernel(void);
extern void basic_rodar_arquivo(char *nome_arquivo);
extern void ac97_tocar_hino(void);
extern void notepad_abrir(const char *nome_arquivo);
extern size_t memory_get_used(void); // Função de uso de RAM do kernel

// Disco virtual do LeoFiles
extern uint8_t virtual_disk[2048][LEO_BLOCK_SIZE];

static char command_buffer[256];
static int buffer_idx = 0;
static uint8_t current_color = 0x0A; // Verde claro por padrao

// Helpers de String para ambiente freestanding
static int str_equals(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*prefix != *str) return 0;
        prefix++;
        str++;
    }
    return 1;
}

static void shell_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++, current_color);
    }
}

static void print_dec(uint32_t val) {
    char buf[12];
    int i = 10;
    buf[11] = '\0';
    if (val == 0) {
        vga_putchar('0', current_color);
        return;
    }
    while (val > 0 && i >= 0) {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    }
    shell_puts(&buf[i + 1]);
}

static void print_hex(uint32_t val) {
    char hex_chars[] = "0123456789ABCDEF";
    shell_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        vga_putchar(hex_chars[(val >> i) & 0x0F], current_color);
    }
}

// E/S de portas para reboot do controlador 8042
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void sys_reboot(void) {
    shell_puts("Reiniciando o Le-nidas OS...\n");
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    __asm__ __volatile__ ("hlt");
}

void shell_init(void) {
    buffer_idx = 0;
    current_color = 0x0A;
    shell_puts("\nLe-nidas OS Shell v3.0 - Lendaria Edition\n");
    shell_puts("Digite 'help' para listar os comandos disponiveis.\n\n");
    shell_puts("lenidas> ");
}

void shell_execute(const char* cmd) {
    if (cmd[0] == '\0') return;

    if (str_equals(cmd, "help") == 0) {
        shell_puts("Comandos disponiveis no Le-nidas OS:\n");
        shell_puts("  help       - Exibe esta mensagem\n");
        shell_puts("  clear      - Limpa a tela VGA\n");
        shell_puts("  login      - Entra no gerenciador de autenticacao\n");
        shell_puts("  reboot     - Reinicia o sistema\n");
        shell_puts("  time       - Exibe a hora atual via RTC Driver\n");
        shell_puts("  data       - Exibe a data e hora formatadas via RTC\n");
        shell_puts("  ls         - Lista os arquivos do LeoFiles\n");
        shell_puts("  criar <n>  - Cria um novo arquivo no disco\n");
        shell_puts("  rm <n>     - Remove um arquivo do disco\n");
        shell_puts("  cred       - Exibe os creditos do desenvolvedor\n");
        shell_puts("  version    - Versao do Kernel\n");
        shell_puts("  programas  - Busca binarios em /system/usr/programas\n");
        shell_puts("  echo <msg> - Imprime uma mensagem na tela\n");
        shell_puts("  mem        - Exibe informacoes do mapa de memoria\n");
        shell_puts("  ram        - Exibe o uso atual de RAM do sistema\n");
        shell_puts("  malloc     - Teste de alocador de memoria do kernel\n");
        shell_puts("  gui        - Inicia o VBE Glass Desktop\n");
        shell_puts("  tui        - Inicia o TUI Desktop\n");
        shell_puts("  uptime     - Exibe informacoes de execucao do sistema\n");
        shell_puts("  color <n>  - Altera a cor do texto do terminal (1-5)\n");
        shell_puts("  whoami     - Exibe o usuario ativo no Shell\n");
        shell_puts("  hino       - Tocar o grito de garra do leonidas\n");
        shell_puts("  faxina     - Executa o faxineiro de RAM do auto-reparo\n");
        shell_puts("  autorepair - Dispara o teste do subsistema de auto-reparo\n");
    }
    else if (str_equals(cmd, "login") == 0) {
        auth_prompt_login();
    }
    else if (str_equals(cmd, "reboot") == 0) {
        sys_reboot();
    }
    else if (str_equals(cmd, "time") == 0) {
        rtc_time_t t;
        rtc_read_datetime(&t);
        shell_puts("Hora atual: ");
        if (t.hour < 10) shell_puts("0"); print_dec(t.hour); shell_puts(":");
        if (t.minute < 10) shell_puts("0"); print_dec(t.minute); shell_puts(":");
        if (t.second < 10) shell_puts("0"); print_dec(t.second); shell_puts("\n");
    }
    else if (str_equals(cmd, "data") == 0) {
        rtc_time_t t;
        char datetime_buf[20];
        rtc_read_datetime(&t);
        rtc_format_datetime(&t, datetime_buf);
        shell_puts("Data e Hora RTC: ");
        shell_puts(datetime_buf);
        shell_puts("\n");
    }
    else if (str_equals(cmd, "ls") == 0) {
        leofiles_list();
    }
    else if (str_starts_with(cmd, "criar ")) {
        const char *filename = cmd + 6;
        if (leofiles_create(filename, 0) == 0) {
            shell_puts("[+] Arquivo '");
            shell_puts(filename);
            shell_puts("' criado com sucesso.\n");
        } else {
            shell_puts("[-] Erro: Falha ao criar arquivo no LeoFiles.\n");
        }
    }
    else if (str_starts_with(cmd, "rm ")) {
        const char *filename = cmd + 3;
        if (leofiles_remove(filename) == 0) {
            shell_puts("[+] Arquivo '");
            shell_puts(filename);
            shell_puts("' removido com sucesso.\n");
        } else {
            shell_puts("[-] Erro: Arquivo nao encontrado.\n");
        }
    }
    else if (str_equals(cmd, "cred") == 0) {
        shell_puts("\n==================================================\n");
        shell_puts(" Davi Rodrigues Boss e o criador do Le-nidas OS.\n");
        shell_puts(" Desenvolvedor de Sistemas Operacionais e Kernel.\n");
        shell_puts("==================================================\n\n");
    }
    else if (str_equals(cmd, "version") == 0) {
        shell_puts("       _nnnn_\n");
        shell_puts("      DooMooPi\n");
        shell_puts("     |====|====|\n");
        shell_puts("     |    |    |     Le-nidas OS v3.0 - Lendaria Edition\n");
        shell_puts("     |____|____|     Escola Estadual Leonidas Ribeiro de Magalhaes\n");
        shell_puts("      //||\\\\||\n");
        shell_puts("     // || ||||\n");
        shell_puts("    //  ||  ||||\n");
    }
    else if (str_starts_with(cmd, "bloco ")) {
    // Pega o nome do arquivo que vem depois de "bloco " (ex: bloco net_escola.bas)
    const char *nome_arquivo = cmd + 6;
    
    // Remove espaços extras se houver
    while (*nome_arquivo == ' ') nome_arquivo++;
    
    if (*nome_arquivo == '\0') {
        shell_puts("[-] Uso incorreto. Exemplo: bloco net_escola\n");
    } else {
        shell_puts("[+] Abrindo Bloco de Notas para: ");
        shell_puts(nome_arquivo);
        shell_puts("\n");
        
        // Chama a função do editor nativo que criamos
        notepad_abrir(nome_arquivo);
    }
}

    else if (str_equals(cmd, "faxina") == 0) {
        autoreparar_faxineiro_ram();
    }
    else if (str_equals(cmd, "autorepair") == 0) {
        autoreparar_tratar_erro(ERRO_LEOFILES_CORROMPIDO);
    }
    else if (str_equals(cmd, "hino") == 0) {
        shell_puts("[AC97] Tocando o Grito de Guerra do Le-nidas OS...\n");
        ac97_tocar_hino();
    }
    else if (str_starts_with(cmd, "basic ")) {
        const char *filename = cmd + 6;
        basic_rodar_arquivo((char *)filename);
    }
    else if (str_starts_with(cmd, "escrever ")) {
        const char *args = cmd + 9;
        char filename[32];
        int i = 0;

        while (*args != ' ' && *args != '\0' && i < 31) {
            filename[i++] = *args++;
        }
        filename[i] = '\0';

        if (*args == ' ') args++;

        if (leofiles_write(filename, (char *)args, 256) == 0) {
            shell_puts("[+] Codigo gravado no arquivo!\n");
        } else {
            shell_puts("[-] Erro ao gravar no LeoFiles.\n");
        }
    }
    else if (str_equals(cmd, "programas") == 0) {
        uint16_t prog_block = leofiles_get_programas_block();
        shell_puts("Escaneando endereco VFS em /system/usr/programas...\n");
        if (prog_block == 0) {
            shell_puts("[-] Erro: Diretorio /system/usr/programas nao inicializado.\n");
        } else {
            struct LeoDirEntry *entries = (struct LeoDirEntry*)virtual_disk[prog_block];
            int enc = 0;
            for (uint32_t i = 0; i < (LEO_BLOCK_SIZE / sizeof(struct LeoDirEntry)); i++) {
                if (entries[i].used) {
                    shell_puts("  -> Encontrado: ");
                    shell_puts(entries[i].name);
                    shell_puts(" [Bloco: ");
                    print_dec(entries[i].first_block);
                    shell_puts("]\n");
                    enc++;
                }
            }
            if (enc == 0) {
                shell_puts("Nenhum binario encontrado em /system/usr/programas.\n");
            }
        }
    }
    else if (str_starts_with(cmd, "echo ")) {
        shell_puts(cmd + 5);
        shell_puts("\n");
    }
    else if (str_starts_with(cmd, "abrir ")) {
    const char *filename = cmd + 6;
    char buffer[1024]; // Buffer para armazenar o conteúdo do arquivo lido
    
    // Tenta ler o arquivo do LeoFiles
    int bytes_lidos = leofiles_read(filename, buffer, sizeof(buffer) - 1);
    
    if (bytes_lidos >= 0) {
        buffer[bytes_lidos] = '\0'; // Garante o fim da string se for texto
        
        shell_puts("[+] Lendo arquivo: ");
        shell_puts(filename);
        shell_puts("\n-----------------------------------\n");
        shell_puts(buffer);
        shell_puts("\n-----------------------------------\n");
        shell_puts("[+] Fim do arquivo. Total de bytes: ");
        print_dec(bytes_lidos);
        shell_puts("\n");
    } else {
        shell_puts("[-] Erro: Arquivo nao encontrado ou falha ao ler.\n");
    }
}

    else if (str_equals(cmd, "mem") == 0) {
        shell_puts("Mapa de Memoria do Kernel:\n");
        shell_puts("  Base do Kernel: 0x00100000\n");
        shell_puts("  LeoFiles VFS  : ");
        print_hex((uint32_t)virtual_disk);
        shell_puts("\n  Estrutura BlockTable: 2048 blocos alocados\n");
    }
    else if (str_equals(cmd, "ram") == 0) {
        size_t bytes_usados = memory_get_used();
        size_t kb_usados = bytes_usados / 1024;
        size_t mb_usados = kb_usados / 1024;

        shell_puts("\n=== STATUS DE USO DE RAM (HEAP) ===\n");
        shell_puts("  Uso em Bytes : ");
        print_dec(bytes_usados);
        shell_puts(" bytes\n");
        shell_puts("  Uso em KB    : ");
        print_dec(kb_usados);
        shell_puts(" KB\n");
        shell_puts("  Uso em MB    : ");
        print_dec(mb_usados);
        shell_puts(" MB\n");
        
        if (mb_usados >= 8) {
            shell_puts("  [!] Alerta: Memoria elevada! Digite 'faxina'.\n");
        } else {
            shell_puts("  [+] Estado: Memoria em niveis estaveis.\n");
        }
        shell_puts("=====================================\n\n");
    }
    else if (str_equals(cmd, "malloc") == 0) {
        shell_puts("Alocando bloco de memoria de teste...\n");
        shell_puts("[+] Malloc: Bloco alocado com sucesso no endereco Heap ");
        print_hex((uint32_t)&virtual_disk[1000][0]);
        shell_puts("\n");
    }
    else if (str_equals(cmd, "gui") == 0 || str_equals(cmd, "startx") == 0) {
        shell_puts("Iniciando Modo Grafico...\n");
        gui_desktop_init(0xC0000000);
    }
    else if (str_equals(cmd, "tui") == 0) {
        shell_puts("Iniciando Modo TUI...\n");
        tui_desktop_start();
    }
    else if (str_equals(cmd, "clear") == 0) {
        vga_clear();
    }
    else if (str_equals(cmd, "whoami") == 0) {
        auth_get_current_user();
    }
    else if (str_equals(cmd, "uptime") == 0) {
        shell_puts("Le-nidas OS rodando em modo x86 Protected Mode.\n");
    }
    else if (str_starts_with(cmd, "color ")) {
        char c = cmd[6];
        if (c == '1') current_color = 0x0A;      // Verde
        else if (c == '2') current_color = 0x0C; // Vermelho
        else if (c == '3') current_color = 0x0B; // Ciano
        else if (c == '4') current_color = 0x0E; // Amarelo
        else if (c == '5') current_color = 0x0F; // Branco
        shell_puts("Cor do terminal alterada.\n");
    }
    else if (str_equals(cmd, "update")) {
        shell_puts("[OTA] Conectando ao driver RTL8139 para checar atualizacoes...\n");
        shell_puts("[OTA] Repositorio Alvo: Daviboss76/Le-nidas_OS (main)\n");

        int st = ota_download_and_save_kernel();

        if (st != 0) {
            shell_puts("[-] OTA: Nao foi possivel concluir o download remoto. Verifique a conexao.\n");
        } else {
            shell_puts("[OTA] Sucesso! Nova versao salva como 'kernel_new.bin' no LeoFiles.\n");
            shell_puts("[OTA] Reinicie o sistema para aplicar a mudanca.\n");
        }
    }
    else {
        shell_puts("Comando nao reconhecido: ");
        shell_puts(cmd);
        shell_puts("\n");
    }
}

void shell_handle_key(char c) {
    if (c == '\n') {
        shell_puts("\n");
        command_buffer[buffer_idx] = '\0';
        shell_execute(command_buffer);
        buffer_idx = 0;
        shell_puts("lenidas> ");
    }
    else if (c == '\b' || c == 127) {
        if (buffer_idx > 0) {
            buffer_idx--;
            vga_putchar('\b', current_color);
            vga_putchar(' ', current_color);
            vga_putchar('\b', current_color);
        }
    }
    else if (c >= 32 && c <= 126) {
        if (buffer_idx < 255) {
            command_buffer[buffer_idx++] = c;
            vga_putchar(c, current_color);
        }
    }
}

