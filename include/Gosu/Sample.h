#pragma once

#include <Gosu/Channel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Sample Gosu_Sample;

Gosu_Sample* Gosu_Sample_create(const char* filename);
void Gosu_Sample_destroy(Gosu_Sample* sample);

Gosu_Channel* Gosu_Sample_play(Gosu_Sample *sample, double volume, double speed, bool looping);
Gosu_Channel* Gosu_Sample_play_pan(Gosu_Sample *sample, double pan, double volume, double speed, bool looping);

#ifdef __cplusplus
}
#endif