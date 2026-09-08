#include "init.h"
#include "vga.h"
#include "leofiles.h"
#include "scheduler.h"
#include "shell.h"
#include "kernel.h"
#include "auth.h"

extern char kbd_getchar(void);

void keyboard_isr_handler(void) {
    char c = kbd_getchar();
    if (!c) return;
    shell_handle_key(c);
}

void kernel_main(void) {
    // 1. Inicializa todo o hardware e drivers do Kernel
    kernel_init_all();

    // 2. Inicializa o Escalonador de Tarefas
    scheduler_init();

    // 3. Executa a tela de Login (com trava e atraso proposital)
    auth_prompt_login();

    // 4. Inicia o Shell apenas apos confirmacao do login
    shell_init();

    // 5. Laco principal do Kernel
    while (1) {
        scheduler_yield();
        __asm__ __volatile__("hlt");
    }
}

