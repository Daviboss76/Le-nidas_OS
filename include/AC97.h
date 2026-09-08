#ifndef AC97_H
#define AC97_H

#include <stdint.h>

// Inicializa a placa de som no barramento PCI
int ac97_init(void);

// Toca um tom de áudio na frequência (Hz) e duração (ms) especificadas
void ac97_play_tone(uint32_t freq_hz, uint32_t duration_ms);

#endif

