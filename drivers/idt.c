// idt.c
#include "idt.h"

struct IDTEntry idt[256];
struct IDTPtr idtp;

void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct IDTEntry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;
    __asm__ __volatile__("lidt %0" : : "m"(idtp));
}

