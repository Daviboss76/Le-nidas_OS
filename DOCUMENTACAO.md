# 📖 Documentação Técnica da API do Kernel — leonidas_OS (v1.0)

**Arquitetura Target:** `i686-pc-none-elf` (x86 32-bit Protected Mode / Ring 0)  
**Compiladores Suportados:** Clang / LLVM (Termux / Linux)  
**Linker:** `ld.lld` (LLVM Linker)

---

## 🛠️ 1. Núcleo e Inicialização (`init.h`)

### `void kernel_init_all(void)`
* **Descrição:** Inicializa os subsistemas essenciais do sistema na ordem correta de dependência de hardware.
* **Fluxo de Inicialização:**
  1. Limpeza do buffer gráfico VGA (`vga_clear`).
  2. Configuração da Tabela Global de Descritores (`gdt_init`).
  3. Remapeamento das IRQs no Controlador de Interrupções (`pic_remap`).
  4. Configuração da IDT (`idt_init`) e registro da IRQ1 (Teclado).
  5. Inicialização do Gerenciador de Heap (`memory_init`).
  6. Ativação global das interrupções da CPU (`sti`).
  7. Carga e execução dos módulos dinâmicos ELF (`modules_init`).
  8. Inicialização do controlador EHCI, Mouse PS/2 e Paginação de Memória (`paging_init`).

---

## 🖥️ 2. Interface de Vídeo VGA (`vga.h`)

O driver opera no modo de texto 80x25 mapeado diretamente no endereço físico `0xB8000`.

### `void vga_clear(void)`
* **Descrição:** Limpa a tela inteira preenchendo os caracteres com espaços e aplica a cor padrão (texto branco sobre fundo preto).

### `void vga_putchar(char c, uint8_t color)`
* **Descrição:** Imprime um único caractere no cursor atual e avança a posição de escrita.
* **Parâmetros:**
  * `c`: Caractere ASCII a ser impresso.
  * `color`: Byte de atributo VGA contendo cor de fundo e texto. Exemplo: `0x0F` (Branco brilhante).

### `void vga_puts(const char *str)`
* **Descrição:** Imprime uma string terminada em nulo (`\0`) no console. Suporta quebra de linha (`\n`).

### `void vga_backspace(void)`
* **Descrição:** Apaga o caractere imediatamente anterior ao cursor atual e recua a posição em 1 coluna.

---

## 🧠 3. Gerenciamento de Memória (`memory.h`)

Gerencia a alocação de memória dinâmica no Heap a partir do endereço físico `0x100000` (1MB).

### `void memory_init(void)`
* **Descrição:** Inicializa os ponteiros de controle e o mapa de blocos do Heap do kernel.

### `void *kmalloc(size_t size)`
* **Descrição:** Reservará um bloco contíguo de bytes no Heap.
* **Parâmetros:** `size` — Quantidade de bytes solicitada.
* **Retorno:** Ponteiro `void*` para o bloco reservado ou `NULL` em caso de falta de memória.

---

## 🧩 4. Carregador de Módulos Dinâmicos LKM (`elf_loader.h`)

Responsável por interpretar, realocar e executar arquivos objeto no formato ELF32 (`.o`) em tempo de execução.

### `int load_module(uint8_t *module_data)`
* **Descrição:** Lê o cabeçalho ELF, aloca espaço na memória via `kmalloc` para as seções `.text`, `.data` e `.bss`, resolve a tabela de símbolos contra o mapa do kernel (`kernel_symbols`) via realocação `R_386_32` e `R_386_PC32`, e executa a função de entrada `module_init()`.
* **Parâmetros:** `module_data` — Ponteiro para o buffer contendo os bytes brutos do arquivo objeto `.o`.
* **Retorno:** `0` em caso de sucesso; valor negativo em caso de falha de validação ou erro de realocação.

### `void modules_init(void)`
* **Descrição:** Função utilitária que invoca o carregador para processar os módulos embutidos no binário durante a inicialização.

---

## 📁 5. Sistema de Arquivos LeoFiles (`leofiles.h`)

Gerenciador de arquivos residente na RAM para registro e organização de arquivos do sistema.

### `void leofiles_format(void)`
* **Descrição:** Inicializa e limpa a tabela de diretórios do sistema LeoFiles.

### `int leofiles_create(const char *name, uint32_t size)`
* **Descrição:** Cria uma entrada de arquivo no diretório do sistema.

### `int leofiles_remove(const char *name)`
* **Descrição:** Remove a entrada de um arquivo existente.

### `void leofiles_list(void)`
* **Descrição:** Itera pela tabela de arquivos e imprime os nomes diretamente na tela VGA.

---

## ⚙️ 6. Escalonador de Tarefas (`kernel.c`)

### `void scheduler_init(void)`
* **Descrição:** Registra a lista de tarefas iniciais no escalonador.

### `void scheduler_yield(void)`
* **Descrição:** Alterna cooperativamente a execução para a próxima tarefa ativa da fila (Round-Robin).

---

## 💻 7. Guia para Desenvolver um Módulo Dinâmico

Para criar um novo módulo dinâmico em C compatível com o carregador do **leonidas_OS**:

1. Crie o arquivo fonte `meu_modulo.c`:
```c
extern void vga_puts(const char *str);

void module_init(void) {
    vga_puts("[MODULO] Meu modulo customizado executou com sucesso!\n");
}

