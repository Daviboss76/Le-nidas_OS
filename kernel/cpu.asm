[BITS 32]

global cpu_enable_interrupts
global cpu_disable_interrupts
global cpu_halt

section .text

cpu_enable_interrupts:
    sti
    ret

cpu_disable_interrupts:
    cli
    ret

cpu_halt:
    hlt
    ret

