// ehci.c
#include "ehci.h"
#include "vga.h"
#include "memory.h"

void ehci_init(uint32_t mmio_base) {
    vga_puts("[+] Inicializando Controlador USB 2.0 (EHCI)...\n");

    volatile struct EHCI_CapRegs *cap = (volatile struct EHCI_CapRegs*)mmio_base;
    volatile struct EHCI_OpRegs  *op  = (volatile struct EHCI_OpRegs*)(mmio_base + cap->caplength);

    // 1. Parar o controlador (limpar bit RS do USBCMD)
    op->usbcmd &= ~0x01;
    while (op->usbsts & (1 << 12) == 0);

    // 2. Reset do Controlador EHCI
    op->usbcmd |= 0x02; // Bit HCRESET
    while ((op->usbsts & (1 << 12)) == 0);

    // 3. Configurar Periodic Frame List Base
    uint32_t *periodic_list = (uint32_t*)kmalloc(4096);
    if (!periodic_list) {
        vga_puts("[-] Erro de alocacao para Periodic List EHCI.\n");
        return;
    }

    for (int i = 0; i < 1024; i++) {
        periodic_list[i] = 0x01; // Bit T (Terminator) ativado = Lista Vazia
    }

    op->periodiclistbase = (uint32_t)periodic_list;

    // 4. Configurar Lista Assíncrona (Async Queue Head fictício para manter a fila)
    struct EHCI_QH *async_head = (struct EHCI_QH*)kmalloc(sizeof(struct EHCI_QH));
    if (async_head) {
        async_head->horizontal_link = ((uint32_t)async_head) | 0x02; // H-bit = QH
        async_head->endpoint_characteristics = (1 << 15); // Head of Reclamation List
        async_head->overlay.next_qtd = 0x01; // T-bit
        async_head->overlay.alt_next_qtd = 0x01;
        
        op->asynclistbase = (uint32_t)async_head;
    }

    // 5. Encaminhar todas as portas para o controlador EHCI
    op->configflag = 0x01; 

    // 6. Iniciar EHCI (RS Bit = 1 e Async Schedule Enable = 1)
    op->usbcmd |= 0x21;

    vga_puts("[+] Driver USB 2.0 (EHCI) ativo e operacional!\n");
}

