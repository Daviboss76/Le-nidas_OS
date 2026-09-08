#include "AC97.h"

// Registradores do Barramento PCI
#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

// Registradores do AC'97 Mixer / Bus Master
#define AC97_MASTER_VOL 0x02
#define AC97_PCM_VOL    0x18
#define AC97_PO_BDBAR   0x10 // Buffer Descriptor Base Address Register
#define AC97_PO_CIV     0x14 // Current Index Value
#define AC97_PO_LVI     0x15 // Last Valid Index
#define AC97_PO_CR      0x1B // Control Register

// Estrutura do Descritor de Buffer DMA (Buffer Descriptor List - BDL)
typedef struct {
    uint32_t pointer;  // Endereço físico do buffer de áudio
    uint16_t samples;  // Número de amostras (máx 65535)
    uint16_t flags;    // Bit 15: Interrupt on Completion (IOC), Bit 14: Buffer Underrun (BUP)
} __attribute__((packed)) ac97_bdl_entry_t;

static uint16_t nabmbar = 0; // Native Audio Bus Master Base Address
static uint16_t nambar  = 0; // Native Audio Mixer Base Address

// Buffers alinhados estaticamente para DMA
static ac97_bdl_entry_t bdl[32] __attribute__((aligned(16)));
static int16_t pcm_buffer[2048] __attribute__((aligned(16)));

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

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

// Varredura PCI para encontrar a placa de som AC'97 (Vendor ID: 0x8086, Device ID: 0x2415)
static int pci_find_ac97(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t dev = 0; dev < 32; dev++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (dev << 11);
            outl(CONFIG_ADDRESS, addr);
            uint32_t vendor_device = inl(CONFIG_DATA);

            // Procure pelo controlador Intel AC'97 (usado por padrão no QEMU)
            if ((vendor_device & 0xFFFF) == 0x8086 && ((vendor_device >> 16) & 0xFFFF) == 0x2415) {
                // Lê NAMBAR (BAR0) e NABMBAR (BAR1)
                outl(CONFIG_ADDRESS, addr | 0x10);
                nambar = inl(CONFIG_DATA) & ~1;

                outl(CONFIG_ADDRESS, addr | 0x14);
                nabmbar = inl(CONFIG_DATA) & ~1;

                // Ativa Bus Mastering e I/O Space no PCI Command Register
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

int ac97_init(void) {
    if (!pci_find_ac97()) {
        return 0; // Placa AC'97 não encontrada
    }

    // Define o volume máximo para o Mixer (Master e PCM Volume)
    outw(nambar + AC97_MASTER_VOL, 0x0000); // 0 = Volume máximo (sem atenuação)
    outw(nambar + AC97_PCM_VOL, 0x0000);

    // Reseta o controlador Bus Master Output
    outb(nabmbar + AC97_PO_CR, 0x02); // Reset bit

    return 1;
}

void ac97_play_tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (!nabmbar) return;

    uint32_t sample_rate = 48000;
    uint32_t total_samples = 2048;

    // Gera onda quadrada PCM 16-bit estéreo
    uint32_t period = sample_rate / freq_hz;
    for (uint32_t i = 0; i < total_samples; i++) {
        int16_t sample = ((i % period) < (period / 2)) ? 8000 : -8000;
        pcm_buffer[i] = sample;
    }

    // Configura a tabela de descritores DMA (BDL)
    bdl[0].pointer = (uint32_t)pcm_buffer;
    bdl[0].samples = total_samples;
    bdl[0].flags = 0x8000; // Interrupt / End of list flag

    // Aponta o BDBAR para a nossa tabela BDL
    outl(nabmbar + AC97_PO_BDBAR, (uint32_t)&bdl[0]);

    // Define o índice final (LVI) como 0
    outb(nabmbar + AC97_PO_LVI, 0);

    // Inicia a reprodução enviando a flag RUN (bit 0) para o Control Register
    outb(nabmbar + AC97_PO_CR, 0x01);

    // Delay simples proporcional à duração (loop de espera)
    for (volatile uint32_t d = 0; d < duration_ms * 10000; d++);

    // Para a reprodução de áudio
    outb(nabmbar + AC97_PO_CR, 0x00);
}

