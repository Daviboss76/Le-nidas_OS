# Le-nidas OS

O **Le-nidas OS** é um sistema operacional x86 de 32 bits rodando em modo protegido, desenvolvido do zero para fins de aprendizado de arquitetura de computadores, kernels monolithic, sistemas de arquivos, drivers de hardware e segurança.

---

## 🏷️ Sistema de Versões
Cada lançamento oficial do **Le-nidas OS** homenageia uma pessoa especial através de seu codinome:
* **Versão Atual:** `v1.8 - Jucilene`

---

## 🚀 Funcionalidades Atuais

* **Core & Kernel:**
  * Bootloader compatível com Multiboot (GRUB / QEMU `-kernel`).
  * Tabela do Descritor Global (**GDT**) e Tabela do Descritor de Interrupções (**IDT**) de 32 bits.
  * Remapeamento do controlador **PIC 8259A** (IRQs `0x20`-`0x2F`).
  * Escalonador de tarefas cooperativas (*Round-Robin*).
  * Gerenciador de memória Heap dinâmica (`kmalloc`).

* **Segurança & Autenticação:**
  * **Gerenciador de Autenticação (`auth`)**: Sistema de login e cadastro de usuários com suporte a mascaramento de senhas (`*`) e tratamento de entrada de teclado via *debounce*.

* **Entrada e Saída (I/O):**
  * Driver de vídeo VGA em modo texto ($80 \times 25$).
  * Driver de Teclado PS/2 com tratamento de repetição e suporte a backspace.
  * Driver de Mouse PS/2 (IRQ12 / Vetor `0x2C`) e interface gráfica/TUI.
  * Controlador USB **EHCI** e driver de áudio **AC97**.

* **Sistema de Arquivos:**
  * **LeoFiles**: Sistema de arquivos virtual com suporte a criação (`criar`), remoção (`rm`), listagem (`ls`) e formatação (`format`).

* **Shell Integrado:**
  * `login`: Abre o gerenciador de autenticação de usuários.
  * `whoami`: Exibe o usuário ativo no sistema.
  * `version`: Exibe a versão do kernel (`1.8 Jucilene`).
  * `ls`, `criar <nome>`, `rm <nome>`: Comandos do LeoFiles.
  * `gui` / `tui`: Inicia as interfaces gráfica e textual.
  * `malloc` / `mem`: Ferramentas de verificação do gerenciador de memória.
  * `clear`: Limpa a tela.
  * `reboot`: Reinicia o sistema via porta `$0x64`.
  * `cred`: Exibe os créditos dos desenvolvedores.

---

## 🛠️ Requisitos de Compilação
* **NASM** (Assembler)
* **Clang / LLVM** (target `i686-pc-none-elf`)
* **LLD** (`ld.lld` - Linker ELF 32-bit)
* **QEMU** (`qemu-system-i386`)

---

## 📦 Como Compilar e Executar
```bash
make clean
make
qemu-system-i386 -kernel kernel.bin -rtc base=localtime -device AC97

