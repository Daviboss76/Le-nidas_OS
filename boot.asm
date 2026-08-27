bits 32
section .text
    align 4
    dd 0x1BADB002              ; Multiboot magic
    dd 0x00                    ; Flags
    dd - (0x1BADB002 + 0x00)   ; Checksum

global _start
global load_page_directory
global enable_paging
global isr33
global isr44
extern kernel_main
extern keyboard_isr_handler
extern mouse_handler

_start:
    cli
    call kernel_main
    hlt

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

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]  ; Endereço da gdtp enviado por parâmetro
    lgdt [eax]           ; Carrega a nova GDT

    mov ax, 0x10        ; Offset do Data Segment (0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush     ; Far jump para recarregar Code Segment (0x08)
.flush:
    ret

load_page_directory:
    mov eax, [esp + 4]  ; Endereço do page_directory passado por parâmetro
    mov cr3, eax        ; Carrega o ponteiro base no CR3
    ret

enable_paging:
    mov eax, cr0
    or eax, 0x80000000  ; Ativa o Bit 31 (PG - Paging Enable)
    mov cr0, eax
    ret
