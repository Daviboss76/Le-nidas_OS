#ifndef AUTOREPARAR_H
#define AUTOREPARAR_H

#include <stdint.h>

#define ERRO_LEOFILES_CORROMPIDO 1
#define ERRO_FALHA_MEMORIA       2
#define ERRO_HEAP_ESGOTADO       3

// Inicializa a tabela de estratégias de reparo
void autoreparar_init(void);

// Função central (Main) que intercepta o código do erro e dispara o reparo
void autoreparar_tratar_erro(int codigo_erro);

// Faxineiro de RAM integrado ao gerenciador de memória
void autoreparar_faxineiro_ram(void);

#endif

