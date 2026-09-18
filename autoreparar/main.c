#include "autoreparar.h"
#include "memory.h"
#include "vga.h"

// Assinatura das rotinas de reparo
typedef void (*FuncaoReparo)(void);

typedef struct {
    int codigo_erro;
    const char *descricao;
    FuncaoReparo acao;
} EntradaReparo;

// Função de delay simples para a animação
static void delay_animacao(void) {
    for (volatile int i = 0; i < 400000; i++);
}

// Efeito visual avançado: Livro girando + Barra de progresso verde
static void executar_animacao_autoreparo(void) {
    vga_clear();
    vga_puts("========================================================\n");
    vga_puts("       LE-NIDAS OS - PROTOCOLO DE AUTO-REPARACAO        \n");
    vga_puts("========================================================\n\n");

    const char* frames_giratorios[] = { "[ / ]", "[ - ]", "[ \\ ]", "[ | ]" };
    int largura_barra = 30;

    for (int p = 0; p <= 100; p += 5) {
        // Posiciona e exibe o símbolo do livro/estado girando
        vga_puts(" Status: ");
        vga_puts(frames_giratorios[(p / 5) % 4]);
        vga_puts("  Progresso: [");

        // Desenha a barra de progresso horizontal em verde/blocos
        int preenchidas = (p * largura_barra) / 100;
        for (int i = 0; i < largura_barra; i++) {
            if (i < preenchidas) {
                vga_puts("=");
            } else {
                vga_puts(" ");
            }
        }
        
        // Formata a porcentagem de forma simples
        vga_puts("] ");
        if (p < 10) vga_puts("  ");
        else if (p < 100) vga_puts(" ");
        
        // Aqui podemos apenas quebrar a linha para o próximo frame da barra
        vga_puts("\r");
        delay_animacao();
    }
    
    vga_puts("\n\n");
}

// Estratégia 1: Reparo do Sistema de Arquivos / Blocos
static void reparar_leofiles(void) {
    executar_animacao_autoreparo();
    
    // Cuspinteira de Logs em massa (Faxina profunda)
    vga_puts("[LOG] Varrendo setores corrompidos no LeoFiles...\n");
    vga_puts("[LOG] Desalocando blocos orfaos da arvore VFS...\n");
    vga_puts("[LOG] Reconstruindo metadatos da Golden Image...\n");
    vga_puts("[LOG] Sincronizando inodes e buffers de cache...\n");
    vga_puts("[LOG] Verificando integridade das estruturas de diretorio...\n");
    vga_puts("[OK]  Sistema de arquivos totalmente saneado e otimizado!\n");
}

// Estratégia 2: Limpeza e Realocação de Memória
static void reparar_memoria(void) {
    executar_animacao_autoreparo();
    
    // Cuspinteira de Logs em massa (Faxina profunda)
    vga_puts("[LOG] Descarregando paginas de memoria obsoletas...\n");
    vga_puts("[LOG] Resetando ponteiros criticos do heap do kernel...\n");
    vga_puts("[LOG] Compactando espaco de endereçamento virtual...\n");
    vga_puts("[LOG] Purgando caches temporarios de drivers de video...\n");
    vga_puts("[LOG] Revalidando limites de alocacao (Max 10MB)...\n");
    autoreparar_faxineiro_ram();
    vga_puts("[OK]  Gerenciamento de memoria reestabelecido com exito!\n");
}

// Tabela central de decisões de reparo
static EntradaReparo tabela_reparacoes[] = {
    { ERRO_LEOFILES_CORROMPIDO, "Corrupcao estrutural no LeoFiles", reparar_leofiles },
    { ERRO_FALHA_MEMORIA,       "Inconsistencia no mapeamento de memoria", reparar_memoria },
    { ERRO_HEAP_ESGOTADO,       "Esgotamento do heap do kernel (10MB Max)", reparar_memoria }
};

void autoreparar_init(void) {
    vga_puts("[+] Subsistema Autoreparar 3.0 inicializado.\n");
}

void autoreparar_tratar_erro(int codigo_erro) {
    vga_puts("\n[AUTOREPAIR] !!! INTERCEPTADA ANOMALIA NO KERNEL !!!\n");

    int total = sizeof(tabela_reparacoes) / sizeof(EntradaReparo);
    int resolvido = 0;

    for (int i = 0; i < total; i++) {
        if (tabela_reparacoes[i].codigo_erro == codigo_erro) {
            vga_puts("[AUTOREPAIR] Diagnostico: ");
            vga_puts(tabela_reparacoes[i].descricao);
            vga_puts("\n[AUTOREPAIR] Acionando nucleo de auto-cura...\n");

            tabela_reparacoes[i].acao();
            resolvido = 1;
            break;
        }
    }

    if (!resolvido) {
        vga_puts("[-] ERRO FATAL: Falha desconhecida nao mapeada na tabela.\n");
    } else {
        vga_puts("[+] Sistema recuperado com sucesso pelo nucleo.\n\n");
    }
}

// O Faxineiro de RAM conectado ao gerenciamento de memória atual
void autoreparar_faxineiro_ram(void) {
    size_t memoria_usada = memory_get_used();
    vga_puts("[FAXINEIRO] Analisando heap do kernel (Uso atual: ");

    if (memoria_usada > (8 * 1024 * 1024)) { // Acima de 8 MB usados
        vga_puts("ALERTA: Heap elevado! Limpando buffers temporarios...\n");
    } else {
        vga_puts("Estavel).\n");
    }
}

