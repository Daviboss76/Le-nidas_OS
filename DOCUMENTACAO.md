# 📖 Documentação Técnica da API do Kernel — Le-nidas OS (v1.8 Jucilene)

**Arquitetura Target:** `i686-pc-none-elf` (x86 32-bit Protected Mode / Ring 0)  
**Compiladores Suportados:** Clang / LLVM (Termux / Linux)  
**Linker:** `ld.lld` (LLVM Linker)  

---

## 🔐 1. Gerenciador de Autenticação (`auth.h`)

Módulo responsável pela identificação, cadastro e controle de sessão de usuários no sistema.

### `void auth_prompt_login(void)`
* **Descrição:** Executa a rotina interativa de login/cadastro no terminal VGA. Se nenhum usuário estiver ativo no sistema, solicita a criação do primeiro usuário e senha (com mascaramento por asteriscos `*`). Se já houver usuário cadastrado, valida as credenciais informadas contra os dados da sessão.
* **Mecanismo Interno:** Utiliza leitura controlada do buffer de teclado (`get_single_key`) para evitar repetição acidental de caracteres.

### `void auth_get_current_user(void)`
* **Descrição:** Imprime no console o nome do usuário autenticado no formato `usuario@lenidas-os`. Caso não haja sessão ativa, exibe `root@lenidas-os (desconectado)`.

---

## 🛠️ 2. Núcleo e Inicialização (`init.h`)

### `void kernel_init_all(void)`
* **Descrição:** Inicializa os subsistemas essenciais do sistema na ordem correta de dependência de hardware.
* **Fluxo de Inicialização:**
  1. Limpeza do buffer gráfico VGA (`vga_clear`).
  2. Configuração da Tabela Global de Descritores (`gdt_init`).
  3. Remapeamento das IRQs no Controlador de Interrupções (`pic_remap`).
  4. Configuração da IDT (`idt_init`) e registro da IRQ1 (Teclado) e IRQ12 (Mouse).
  5. Inicialização do Gerenciador de Heap (`memory_init`).
  6. Ativação global das interrupções da CPU (`sti`).
  7. Carga de módulos dinâmicos ELF (`modules_init`).
  8. Inicialização do controlador EHCI, suporte AC97 e Paginação (`paging_init`).

---

## 🖥️ 3. Interface de Vídeo VGA (`vga.h`)

O driver opera no modo de texto 80x25 mapeado no endereço físico `0xB8000`.

### `void vga_clear(void)`
* **Descrição:** Limpa a tela preenchendo com espaços e aplica a cor padrão.

### `void vga_putchar(char c, uint8_t color)`
* **Descrição:** Imprime um caractere na posição atual do cursor.

### `void vga_puts(const char *str)`
* **Descrição:** Imprime uma string terminada em nulo (`\0`).

### `void vga_backspace(void)`
* **Descrição:** Remove o caractere anterior e recua o cursor em uma coluna.

---

## 🧠 4. Gerenciamento de Memória (`memory.h`)

Gerencia a alocação de memória dinâmica no Heap a partir de `0x100000` (1MB).

### `void memory_init(void)`
* **Descrição:** Inicializa os ponteiros de controle e o mapa de blocos do Heap.

### `void *kmalloc(size_t size)`
* **Descrição:** Aloca um bloco contíguo de bytes no Heap.

---

## 📁 5. Sistema de Arquivos LeoFiles (`leofiles.h`)

### `void leofiles_format(void)`
* **Descrição:** Formata e limpa a tabela de diretórios residente em RAM.

### `int leofiles_create(const char *name, uint32_t size)`
* **Descrição:** Cria uma entrada de arquivo na tabela.

### `int leofiles_remove(const char *name)`
* **Descrição:** Remove uma entrada existente.

### `void leofiles_list(void)`
* **Descrição:** Lista os arquivos cadastrados na tela.

---

## ⚙️ 6. Escalonador de Tarefas (`scheduler.h`)

### `void scheduler_init(void)`
* **Descrição:** Registra a lista de tarefas iniciais no escalonador.

### `void scheduler_yield(void)`
* **Descrição:** Alterna a execução cooperativa para a próxima tarefa (*Round-Robin*).

