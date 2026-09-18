# Le-nidas OS

O **Le-nidas OS** é um sistema operacional x86 de 32 bits rodando em modo protegido, desenvolvido do zero para fins de aprendizado de arquitetura de computadores, kernels monolíticos, sistemas de arquivos, drivers de hardware e interpretadores.

---

## 🏷️ Sistema de Versões
Cada lançamento oficial do **Le-nidas OS** homenageia uma pessoa especial através de seu codinome:
* **Versão Atual:** `v3.0 Beta 1` *(Em desenvolvimento ativo)*

> ⚠️ **Nota:** A versão 3.0 está atualmente em fase **Beta 1**. Algumas funcionalidades, utilitários e drivers ainda estão passando por refinamentos e testes de estabilidade.

---

## 🚀 Novidades e Funcionalidades Atuais

* **Linguagens & Utilitários Embutidos (Novo!):**
  * **Interpretador BASIC (`basic_rodar_arquivo`):** Execução de scripts BASIC armazenados no sistema de arquivos com comandos de exibição (`PRINT`), atribuição de variáveis e operações aritméticas.
  * **Bloco de Notas Nativo (`bloco`):** Editor de texto integrado para criação, edição e visualização de arquivos diretamente pelo terminal.

* **Rede & Atualizações (Novo!):**
  * **Módulo OTA (Over-The-Air):** Sistema de atualização remota integrado via driver de rede **RTL8139**, preparado para sincronização com repositórios no GitHub.

* **Core & Kernel:**
  * Bootloader compatível com Multiboot (GRUB / QEMU `-kernel`).
  * Tabela do Descritor Global (**GDT**) e Tabela do Descritor de Interrupções (**IDT**) de 32 bits.
  * Remapeamento do controlador **PIC 8259A** (IRQs `0x20`-`0x2F`).
  * Escalonador de tarefas cooperativas (*Round-Robin*).
  * Gerenciador de memória Heap dinâmica (`kmalloc`).

* **Segurança & Autenticação:**
  * **Gerenciador de Autenticação (`auth`)**: Sistema de login e cadastro de usuários com suporte a mascaramento de senhas (`*`) e tratamento de entrada de teclado via *debounce*.

* **Entrada, Saída (I/O) & Multimídia:**
  * Driver de vídeo VGA em modo texto ($80 \times 25$) e suporte a modo gráfico VBE (**VBE Glass**).
  * Driver de Teclado PS/2 com tratamento de repetição e suporte a backspace.
  * Driver de Mouse PS/2 (IRQ12 / Vetor `0x2C`) e interface gráfica/TUI.
  * Controlador USB **EHCI**, placa de rede **RTL8139** e driver de áudio **AC97** (com reprodução de áudio binário).

* **Sistema de Arquivos:**
  * **LeoFiles**: Sistema de arquivos virtual (VFS) modular com suporte a operações de núcleo, criação (`criar`), remoção (`rm`), leitura/escrita e formatação (`format`).

* **Shell Integrado:**
  * `bloco <arquivo>`: Abre o Bloco de Notas para criar/editar arquivos.
  * `basic <arquivo>`: Executa um script escrito em BASIC.
  * `login`: Abre o gerenciador de autenticação de usuários.
  * `whoami`: Exibe o usuário ativo no sistema.
  * `version`: Exibe a versão do kernel (`v3.0 Beta 1`).
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

No ambiente Termux ou Linux, execute:

```bash
make clean
make
qemu-system-i386 -kernel kernel.bin -rtc base=localtime -device AC97 -net nic,model=rtl8139

