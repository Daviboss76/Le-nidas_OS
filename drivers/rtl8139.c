#include "rtl8139.h"
#include "vga.h"
#include "idt.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

#define RTL_REG_RBSTART    0x30
#define RTL_REG_CR         0x37
#define RTL_REG_IMR        0x3C
#define RTL_REG_ISR        0x3E
#define RTL_REG_RCR        0x44
#define RTL_REG_CONFIG1    0x52

#define RTL_CR_RESET       0x10
#define RTL_CR_RE          0x08
#define RTL_CR_TE          0x04

static uint32_t rtl_io_base = 0;
static uint8_t rx_buffer[8192 + 16 + 1500] __attribute__((aligned(16)));
static uint16_t rx_offset = 0;

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ __volatile__ ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static int pci_find_rtl8139(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t dev = 0; dev < 32; dev++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (dev << 11);
            outl(CONFIG_ADDRESS, addr);
            uint32_t vendor_device = inl(CONFIG_DATA);

            if ((vendor_device & 0xFFFF) == 0x10EC && ((vendor_device >> 16) & 0xFFFF) == 0x8139) {
                outl(CONFIG_ADDRESS, addr | 0x10);
                rtl_io_base = inl(CONFIG_DATA) & ~3;

                outl(CONFIG_ADDRESS, addr | 0x04);
                uint32_t cmd = inl(CONFIG_DATA);
                outl(CONFIG_ADDRESS, addr | 0x04);
                outl(CONFIG_DATA, cmd | 0x05);

                return 1;
            }
        }
    }
    return 0;
}

void rtl8139_init(void) {
    vga_puts("[RTL8139] Procurando placa de rede PCI...\n");

    if (!pci_find_rtl8139()) {
        vga_puts("[RTL8139] Aviso: Placa nao encontrada no barramento.\n");
        return;
    }

    vga_puts("[RTL8139] Placa de rede encontrada!\n");

    outb(rtl_io_base + RTL_REG_CONFIG1, 0x00);

    outb(rtl_io_base + RTL_REG_CR, RTL_CR_RESET);
    while ((inb(rtl_io_base + RTL_REG_CR) & RTL_CR_RESET) != 0);

    outl(rtl_io_base + RTL_REG_RBSTART, (uint32_t)rx_buffer);
    outl(rtl_io_base + RTL_REG_RCR, 0xF | (1 << 7));
    outb(rtl_io_base + RTL_REG_CR, RTL_CR_RE | RTL_CR_TE);
    outw(rtl_io_base + RTL_REG_IMR, 0x0005);

    vga_puts("[RTL8139] Driver inicializado com sucesso!\n");
}

void rtl8139_send_packet(void *data, uint16_t len) {
    if (!rtl_io_base) return;
    outl(rtl_io_base + 0x20, (uint32_t)data);
    outl(rtl_io_base + 0x10, len);
}

int rtl8139_receive_packet(uint8_t *buf, uint32_t max_size) {
    if (!rtl_io_base) return -1;

    uint16_t *tsad = (uint16_t *)(rx_buffer + rx_offset);
    uint16_t length = *(tsad + 1);

    if (length == 0 || length > 1792) {
        return 0;
    }

    uint8_t *packet_data = (uint8_t *)(tsad + 2);
    uint32_t copy_len = (length < max_size) ? length : max_size;

    for (uint32_t i = 0; i < copy_len; i++) {
        buf[i] = packet_data[i];
    }

    rx_offset = (rx_offset + length + 4 + 3) & ~3;
    if (rx_offset > 8192) {
        rx_offset = 0;
    }

    return copy_len;
}

void rtl8139_handler(void) {
    uint16_t status = inw(rtl_io_base + RTL_REG_ISR);
    if (status & 0x01) {
        vga_puts("[RTL8139] Pacote recebido!\n");
    }
    outw(rtl_io_base + RTL_REG_ISR, status);
}

