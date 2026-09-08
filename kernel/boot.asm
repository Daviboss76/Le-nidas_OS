bits 32

; Constantes Multiboot
MULTIBOOT_PAGE_ALIGN       equ 1 << 0    ; Alinha módulos em páginas
MULTIBOOT_MEMORY_INFO      equ 1 << 1    ; Fornece mapa de memória
MULTIBOOT_GRAPHICS_MODE    equ 1 << 2    ; Solicita modo gráfico VBE
MULTIBOOT_HEADER_MAGIC     equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS     equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_GRAPHICS_MODE
MULTIBOOT_CHECKSUM         equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

section .multiboot
    align 4
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; Modo de Vídeo Solicitado: 0 = Gráfico (Linear Framebuffer)
    dd 0    ; Mode_type (0 = linear graphics)
    dd 1024 ; Width
    dd 768  ; Height
    dd 32   ; BPP (Bits Per Pixel)

section .text

global _start
global load_page_directory
global enable_paging
global isr33
global isr44
global gdt_flush

extern kernel_main
extern keyboard_isr_handler
extern mouse_handler

_start:
    cli

    ; Passa o ponteiro multiboot_info_t em EBX para a kernel_main
    push ebx

    call kernel_main

.loop:
    hlt
    jmp .loop

isr33:
    pusha
    call keyboard_isr_handler
    popa
    iret

isr44:
    pusha
    call mouse_handler
    popa
    iret

gdt_flush:
    mov eax, [esp + 4]  ; Endereço da GDTR enviado por parâmetro
    lgdt [eax]          ; Carrega a nova GDT

    mov ax, 0x10        ; Offset do Data Segment (0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush     ; Far jump para atualizar o CS (0x08)
.flush:
    ret

load_page_directory:
    mov eax, [esp + 4]  ; Endereço do Page Directory
    mov cr3, eax        ; Carrega no registrador CR3
    ret

enable_paging:
    mov eax, cr0
    or eax, 0x80000000  ; Ativa o bit PG (Paging)
    mov cr0, eax
    ret

