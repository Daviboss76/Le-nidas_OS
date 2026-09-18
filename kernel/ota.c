#include "leofiles.h"
#include "rtl8139.h"

#define OTA_MAX_BUFFER_SIZE 16384
#define OTA_SERVER_IP "10.0.2.2"
#define OTA_SERVER_PORT 8080

static unsigned char ota_download_buffer[OTA_MAX_BUFFER_SIZE];
static uint32_t total_downloaded_bytes = 0;

int ota_download_and_save_kernel(void) {
    total_downloaded_bytes = 0;

    // 1. Loop de recepção dos bytes vindos do driver RTL8139 (apontando para o gateway 10.0.2.2:8080 do host)
    while (total_downloaded_bytes < OTA_MAX_BUFFER_SIZE) {
        int received = rtl8139_receive_packet(
            ota_download_buffer + total_downloaded_bytes,
            OTA_MAX_BUFFER_SIZE - total_downloaded_bytes
        );

        if (received <= 0) {
            break;
        }

        total_downloaded_bytes += received;
    }

    if (total_downloaded_bytes == 0) {
        return -1; // [-] Nenhum dado foi baixado da rede.
    }

    // 2. Remove versão anterior se existir para evitar conflito
    leofiles_remove("kernel_new.bin");

    // 3. Cria o arquivo passando o nome e o tipo (0 para arquivo comum)
    int create_res = leofiles_create("kernel_new.bin", 0);
    if (create_res < 0) {
        return -2; // [-] Erro crítico: Não foi possível criar o arquivo no LeoFiles.
    }

    // 4. Escreve os bytes com o cast correto para const char *
    int write_res = leofiles_write("kernel_new.bin", (const char *)ota_download_buffer, total_downloaded_bytes);
    if (write_res < 0) {
        return -3; // [OTA] Erro ao gravar blocos no LeoFiles.
    }

    return 0; // [OTA] Sucesso! Nova versão salva como 'kernel_new.bin' no LeoFiles.
}

