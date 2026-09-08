#include "auth.h"
#include "vga.h"
#include "leofiles.h"

extern char kbd_getchar(void);

typedef struct {
    char username[32];
    char password[32];
    uint8_t active;
} user_record_t;

static user_record_t current_user = {0};
static int is_logged_in = 0;

// Função auxiliar para esperar a tecla ser solta e evitar o disparo de 'dddd'
static char get_single_key(void) {
    char c = 0;
    while ((c = kbd_getchar()) == 0); // Espera apertar
    
    // Pequeno atraso para dar tempo de soltar a tecla
    for (volatile int i = 0; i < 500000; i++) {
        __asm__ __volatile__("nop");
    }
    return c;
}

static void read_input_line(char* buf, int max_len, uint8_t mask_stars) {
    int idx = 0;
    while (1) {
        char c = get_single_key();

        if (c == '\n' || c == '\r') {
            vga_putchar('\n', 0x0F);
            buf[idx] = '\0';
            break;
        } 
        else if (c == '\b' || c == 127) {
            if (idx > 0) {
                idx--;
                vga_puts("\b \b");
            }
        } 
        else if (c >= 32 && c <= 126) {
            if (idx < max_len - 1) {
                buf[idx++] = c;
                if (mask_stars) {
                    vga_putchar('*', 0x0F);
                } else {
                    vga_putchar(c, 0x0F);
                }
            }
        }
    }
}

static int str_cmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

static void str_copy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

void auth_prompt_login(void) {
    char user_in[32];
    char pass_in[32];

    vga_puts("\n--- GERENCIADOR DE AUTENTICACAO ---\n");

    if (!current_user.active) {
        vga_puts("[+] Nenhum usuario cadastrado.\n");
        vga_puts("Novo Usuario: ");
        read_input_line(user_in, 32, 0);

        vga_puts("Nova Senha: ");
        read_input_line(pass_in, 32, 1);

        str_copy(current_user.username, user_in);
        str_copy(current_user.password, pass_in);
        current_user.active = 1;
        is_logged_in = 1;

        vga_puts("\n[+] Usuario '");
        vga_puts(current_user.username);
        vga_puts("' cadastrado e autenticado com sucesso!\n\n");
    } else {
        vga_puts("Login: ");
        read_input_line(user_in, 32, 0);

        vga_puts("Senha: ");
        read_input_line(pass_in, 32, 1);

        if (str_cmp(user_in, current_user.username) == 0 &&
            str_cmp(pass_in, current_user.password) == 0) {
            is_logged_in = 1;
            vga_puts("\n[+] Login efetuado com sucesso! Bem-vindo, ");
            vga_puts(current_user.username);
            vga_puts(".\n\n");
        } else {
            vga_puts("\n[-] Usuario ou senha incorretos.\n\n");
        }
    }
}

void auth_get_current_user(void) {
    if (is_logged_in && current_user.active) {
        vga_puts(current_user.username);
        vga_puts("@lenidas-os\n");
    } else {
        vga_puts("root@lenidas-os (desconectado)\n");
    }
}

