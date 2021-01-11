#pragma once

#include "Gosu_Channel.h"
#include "Gosu_FFI.h"

typedef struct Gosu_Sample Gosu_Sample;

GOSU_FFI_API Gosu_Sample* Gosu_Sample_create(const char* filename);
GOSU_FFI_API void Gosu_Sample_destroy(Gosu_Sample* sample);

GOSU_FFI_API Gosu_Channel* Gosu_Sample_play(Gosu_Sample* sample, double volume, double speed,
                                            bool looping);
GOSU_FFI_API Gosu_Channel* Gosu_Sample_play_pan(Gosu_Sample* sample, double pan, double volume,
                                                double speed, bool looping);
