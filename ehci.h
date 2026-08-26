// ehci.h
#ifndef EHCI_H
#define EHCI_H

#include <stdint.h>

// Registradores de Capacidade EHCI
struct EHCI_CapRegs {
    uint8_t  caplength;
    uint8_t  reserved;
    uint16_t hciversion;
    uint32_t hcsparams;
    uint32_t hccparams;
} __attribute__((packed));

// Registradores Operacionais EHCI
struct EHCI_OpRegs {
    uint32_t usbcmd;
    uint32_t usbsts;
    uint32_t usbintr;
    uint32_t frindex;
    uint32_t ctrldssegment;
    uint32_t periodiclistbase;
    uint32_t asynclistbase;
    uint32_t reserved[9];
    uint32_t configflag;
    uint32_t portsc[1]; // Variável de acordo com HCSPARAMS
} __attribute__((packed));

// Queue Element Transfer Descriptor (qTD) - USB 2.0
struct EHCI_qTD {
    uint32_t next_qtd;
    uint32_t alt_next_qtd;
    uint32_t token;
    uint32_t buffer[5];
} __attribute__((packed, aligned(32)));

// Queue Head (QH) - USB 2.0
struct EHCI_QH {
    uint32_t horizontal_link;
    uint32_t endpoint_characteristics;
    uint32_t endpoint_capabilities;
    uint32_t current_qtd;
    struct EHCI_qTD overlay;
} __attribute__((packed, aligned(32)));

void ehci_init(uint32_t mmio_base);

#endif

