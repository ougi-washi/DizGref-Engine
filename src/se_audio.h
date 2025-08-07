// Syphax-Engine - Ougi Washi

#ifndef SE_AUDIO_H
#define SE_AUDIO_H

#include "se_math.h"
#include <portaudio.h>

void se_audio_input_init();
void se_audio_input_cleanup();
se_vec3 se_audio_input_get_amplitudes();

#endif // SE_AUDIO_H
