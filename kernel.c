// kernel.c
#include "init.h"
#include "vga.h"
#include "leofiles.h"
#include "memory.h"
#include "elf_loader.h"

extern char kbd_getchar(void);

static char buffer[128];
static uint8_t buf_idx = 0;

// Estrutura para o Escalonador Simples
typedef void (*task_func_t)(void);

typedef struct {
    uint32_t id;
    task_func_t func;
    uint8_t active;
} Task;

#define MAX_TASKS 3
static Task task_list[MAX_TASKS];
static uint8_t current_task = 0;

// Tarefas de exemplo para o escalonador
void task_a(void) { }
void task_b(void) { }

void scheduler_init(void) {
    task_list[0] = (Task){1, task_a, 1};
    task_list[1] = (Task){2, task_b, 1};
    task_list[2] = (Task){0, 0, 0};
}

void scheduler_yield(void) {
    current_task = (current_task + 1) % MAX_TASKS;
    if (task_list[current_task].active && task_list[current_task].func) {
        task_list[current_task].func();
    }
}

// Funções auxiliares de string
static int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

static int strncmp(const char *a, const char *b, int n) {
    while (n && *a && (*a == *b)) {
        a++; b++; n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
}

// Processador de Comandos
void execute_command(void) {
    vga_putchar('\n', 0x0F);

    if (strcmp(buffer, "version") == 0) {
        vga_puts("leonidas_OS v1.0 (x86 32-bit Real-Mode/Protected Kernel)\n");
    }
    else if (strcmp(buffer, "uname") == 0) {
        vga_puts("leonidas_OS i686-pc-none-elf x86_32 Ring-0\n");
    }
    else if (strcmp(buffer, "date") == 0) {
        vga_puts("Data de compilacao do kernel: " __DATE__ " as " __TIME__ "\n");
    }
    else if (strcmp(buffer, "ls") == 0) {
        leofiles_list();
    }
    else if (strcmp(buffer, "mod") == 0) {
        vga_puts("[LKM] Carregando modulo dinamicamente...\n");
        modules_init();
    }
    else if (strncmp(buffer, "echo ", 5) == 0) {
        vga_puts(buffer + 5);
        vga_puts("\n");
    }
    else if (strcmp(buffer, "malloc") == 0) {
        char *ptr = (char*)kmalloc(64);
        if (ptr) {
            vga_puts("[+] Alocacao realizada com sucesso! Bloco de 64 bytes reservado no Heap.\n");
        } else {
            vga_puts("[-] Erro ao alocar memoria.\n");
        }
    }
    else if (strncmp(buffer, "criar ", 6) == 0) {
        char *filename = buffer + 6;
        if (leofiles_create(filename, 0) == 0) {
            vga_puts("[+] Arquivo criado: ");
            vga_puts(filename);
            vga_puts("\n");
        } else {
            vga_puts("[-] Erro ao criar arquivo.\n");
        }
    }
    else if (strncmp(buffer, "rm ", 3) == 0) {
        char *filename = buffer + 3;
        if (leofiles_remove(filename) == 0) {
            vga_puts("[+] Arquivo removido: ");
            vga_puts(filename);
            vga_puts("\n");
        } else {
            vga_puts("[-] Arquivo nao encontrado.\n");
        }
    }
    else if (strcmp(buffer, "mem") == 0) {
        vga_puts("Status da Memoria:\n");
        vga_puts("- Heap Base: 0x100000 (1MB)\n");
        vga_puts("- Status: Gerenciador de memoria ativo\n");
    }
    else if (strcmp(buffer, "reboot") == 0) {
        vga_puts("Reiniciando o sistema...\n");
        uint8_t good = 0x02;
        while (good & 0x02) {
            __asm__ __volatile__("inb $0x64, %0" : "=a"(good));
        }
        __asm__ __volatile__("outb %0, $0x64" : : "a"((uint8_t)0xFE));
    }
    else if (strcmp(buffer, "halt") == 0) {
        vga_puts("Sistema desativado com seguranca. Pode fechar o emulator.\n");
        while (1) {
            __asm__ __volatile__("cli; hlt");
        }
    }
    else if (strcmp(buffer, "clear") == 0) {
        vga_clear();
    }
    else if (strcmp(buffer, "cred") == 0) {
        vga_puts("Criador: Davi Rodrigues Boss\n");
        vga_puts("Estudante do 6 ano B na Escola Estadual Leonidas Ribeiro de Magalhaes.\n");
        vga_puts("Davi tem 12 anos e ama o leonidas_OS!\n");
    }
    else if (strcmp(buffer, "help") == 0) {
        vga_puts("Comandos: version, uname, date, ls, mod, echo <txt>, malloc, criar <nome>, rm <nome>, mem, reboot, halt, clear, cred\n");
    }
    else if (buf_idx > 0) {
        vga_puts("Comando desconhecido: ");
        vga_puts(buffer);
        vga_puts("\nDigite 'help' para listar os comandos.\n");
    }

    buf_idx = 0;
    buffer[0] = 0;
    vga_puts("leonidas> ");
}

void keyboard_isr_handler(void) {
    char c = kbd_getchar();
    if (!c) return;

    if (c == '\n') {
        buffer[buf_idx] = '\0';
        execute_command();
    } else if (c == '\b') {
        if (buf_idx > 0) {
            buf_idx--;
            vga_backspace();
        }
    } else if (buf_idx < 127) {
        buffer[buf_idx++] = c;
        vga_putchar(c, 0x0F);
    }
}

void kernel_main(void) {
    kernel_init_all();
    scheduler_init();
    leofiles_format();

    vga_puts("leonidas_OS com Escalonador Ativo!\n");
    vga_puts("Digite 'cred' para ver o criador ou 'help' para comandos.\n\n");
    vga_puts("leonidas> ");

    while (1) {
        scheduler_yield();
        __asm__ __volatile__("hlt");
    }
}

