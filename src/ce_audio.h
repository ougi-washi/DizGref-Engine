// Capsian-Engine - Ougi Washi

#ifndef CE_AUDIO_H
#define CE_AUDIO_H

#include "ce_math.h"
#include <portaudio.h>

void ce_audio_input_init();
void ce_audio_input_cleanup();
ce_vec3 ce_audio_input_get_amplitudes();

#endif // CE_AUDIO_H
