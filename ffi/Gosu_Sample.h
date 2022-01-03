#pragma once

#include "Gosu_Channel.h"
#include "Gosu_FFI.h"

typedef struct Gosu_Sound Gosu_Sample;

GOSU_FFI_API Gosu_Sound* Gosu_Sound_create(const char* filename);
GOSU_FFI_API void Gosu_Sound_destroy(Gosu_Sound* sound);

GOSU_FFI_API Gosu_Channel* Gosu_Sound_play(Gosu_Sound* sound, double volume, double speed,
                                           bool looping);
GOSU_FFI_API Gosu_Channel* Gosu_Sound_play_pan(Gosu_Sound* sound, double pan, double volume,
                                               double speed, bool looping);
