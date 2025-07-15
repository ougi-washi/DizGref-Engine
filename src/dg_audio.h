// DizGref-Engine - Ougi Washi

#ifndef DG_AUDIO_H
#define DG_AUDIO_H

#include "dg_math.h"
#include <portaudio.h>

void dg_audio_init();
void dg_audio_cleanup();
dg_vec3 dg_audio_get_amplitudes();

#endif // DG_AUDIO_H
