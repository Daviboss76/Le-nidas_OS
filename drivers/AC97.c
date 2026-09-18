#include <stdint.h>

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
    uint16_t flags;    // Bit 15: Interrupt on Completion (IOC)
} __attribute__((packed)) ac97_bdl_entry_t;

static uint16_t nabmbar = 0; // Native Audio Bus Master Base Address
static uint16_t nambar  = 0; // Native Audio Mixer Base Address

// Buffer BDL alinhado
static ac97_bdl_entry_t bdl[32] __attribute__((aligned(16)));

// Símbolos gerados pelo objcopy do arquivo audio_hino.bin
extern char _binary_audio_hino_bin_start[];
extern char _binary_audio_hino_bin_end[];

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

// Varredura PCI para encontrar a placa de som AC'97
static int pci_find_ac97(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t dev = 0; dev < 32; dev++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (dev << 11);
            outl(CONFIG_ADDRESS, addr);
            uint32_t vendor_device = inl(CONFIG_DATA);

            if ((vendor_device & 0xFFFF) == 0x8086 && ((vendor_device >> 16) & 0xFFFF) == 0x2415) {
                outl(CONFIG_ADDRESS, addr | 0x10);
                nambar = inl(CONFIG_DATA) & ~1;

                outl(CONFIG_ADDRESS, addr | 0x14);
                nabmbar = inl(CONFIG_DATA) & ~1;

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
        return 0;
    }

    // Configura o volume principal e PCM no mixer (sem Mute / volume alto)
    outw(nambar + AC97_MASTER_VOL, 0x0000);
    outw(nambar + AC97_PCM_VOL, 0x0000);
    
    // Reseta o controlador Bus Master de saída PCM (PCM Out)
    outb(nabmbar + AC97_PO_CR, 0x02);

    return 1;
}

// Função que toca a gravação original convertida via DMA do AC'97
void ac97_tocar_hino(void) {
    if (!nabmbar) return;

    uint32_t audio_start = (uint32_t)_binary_audio_hino_bin_start;
    uint32_t audio_len = (uint32_t)(_binary_audio_hino_bin_end - _binary_audio_hino_bin_start);

    // Configura o Sample Rate para 22050 Hz no registrador do AC'97 (0x2C)
    outw(nambar + 0x2C, 22050);

    uint32_t bytes_enviados = 0;
    uint32_t bloco_max = 65530; // Limite máximo de samples por descritor BDL

    while (bytes_enviados < audio_len) {
        uint32_t tamanho_atual = audio_len - bytes_enviados;
        if (tamanho_atual > bloco_max) tamanho_atual = bloco_max;

        // Configura o descritor DMA BDL
        bdl[0].pointer = audio_start + bytes_enviados;
        bdl[0].samples = (uint16_t)(tamanho_atual / 2); // 16-bit por sample (2 bytes)
        bdl[0].flags = 0x8000;                         // IOC (Interrupt on Completion)

        // Envia o descritor para os registradores de DMA do AC'97
        outl(nabmbar + AC97_PO_BDBAR, (uint32_t)&bdl[0]);
        outb(nabmbar + AC97_PO_LVI, 0);
        outb(nabmbar + AC97_PO_CR, 0x01); // Inicia a reprodução (RUN = 1)

        // Aguarda a reprodução deste bloco terminar na controladora
        while ((inb(nabmbar + 0x16) & 0x01) == 0);

        bytes_enviados += tamanho_atual;
    }

    // Para o canal PCM Out após reproduzir todo o áudio
    outb(nabmbar + AC97_PO_CR, 0x00);
}

    // Compatibilidade para o init.c que ainda chama ac97_play_tone
void ac97_play_tone(uint32_t freq, uint32_t duration) {
    (void)freq;
    (void)duration;
    ac97_tocar_hino();
}

