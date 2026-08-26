# leonidas_OS

O **leonidas_OS** é um sistema operacional x86 de 32 bits rodando em modo protegido, desenvolvido do zero para fins de aprendizado de arquitetura de computadores, kernels monolíticos, sistemas de arquivos e drivers de hardware.

---

## 🚀 Funcionalidades Atuais

* **Core & Kernel:**
  * Bootloader compatível com Multiboot (GRUB / QEMU `-kernel`).
  * Tabela do Descritor Global (**GDT**) e Tabela do Descritor de Interrupções (**IDT**) de 32 bits.
  * Remapeamento do controlador **PIC 8259A** (IRQs `0x20`-`0x2F`).
  * Escalonador simples de tarefas cooperativas (*Round-Robin*).
  * Gerenciador de memória Heap dinâmica (`kmalloc`).

* **Entrada e Saída (I/O):**
  * Driver de vídeo VGA em modo texto ($80 \times 25$).
  * Driver de Teclado PS/2 com mapeamento de caracteres e suporte a backspace.
  * Driver de Mouse PS/2 (IRQ12 / Vetor `0x2C`).
  * Controlador USB **EHCI** (emulação/MMIO).

* **Sistema de Arquivos:**
  * **LeoFiles**: Sistema de arquivos virtual customizado com suporte a criação (`criar`), remoção (`rm`), listagem (`ls`) e formatação (`format`).

* **Shell Integrado:**
  * `version`: Exibe a versão do sistema.
  * `ls`: Lista os arquivos salvos no LeoFiles.
  * `criar <nome>`: Cria um novo arquivo.
  * `rm <nome>`: Remove um arquivo existente.
  * `malloc`: Testa alocação dinâmica de memória na Heap.
  * `mem`: Exibe informações da memória física/Heap.
  * `clear`: Limpa o terminal VGA.
  * `reboot`: Reinicia a máquina física ou virtual via porta do teclado ($0\times64$).
  * `cred`: Exibe as credenciais do criador do sistema.

---

## 🛠️ Requisitos de Compilação

Para compilar e rodar o projeto, é necessário possuir a toolchain x86 configurada:

* **NASM** (Assembler)
* **Clang / LLVM** (com suporte ao target `i686-pc-none-elf`)
* **LLD** (`ld.lld` - Linker ELF 32-bit)
* **QEMU** (`qemu-system-i386`)

---

## 📦 Como Compilar e Executar

### 1. Recompilar os Arquivos Objeto

```bash
# Assembly
nasm -f elf32 boot.asm -o boot.o

# Módulos em C
clang -target i686-pc-none-elf -ffreestanding -c gdt.c -o gdt.o
clang -target i686-pc-none-elf -ffreestanding -c idt.c -o idt.o
clang -target i686-pc-none-elf -ffreestanding -c pic.c -o pic.o
clang -target i686-pc-none-elf -ffreestanding -c keyboard.c -o keyboard.o
clang -target i686-pc-none-elf -ffreestanding -c mouse.c -o mouse.o
clang -target i686-pc-none-elf -ffreestanding -c vga.c -o vga.o
clang -target i686

