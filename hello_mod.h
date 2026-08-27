#ifndef HELLO_MOD_H
#define HELLO_MOD_H

#include <stdint.h>

// Gera o array diretamente quando o header é incluído
static const unsigned char hello_mod_o[] = {
#include "hello_mod_bytes.h"
};

#endif

