#pragma once

#include "Gosu_FFI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Channel Gosu_Channel;

GOSU_FFI_API Gosu_Channel* Gosu_Channel_create(Gosu_Channel* channel);
GOSU_FFI_API void Gosu_Channel_destroy(Gosu_Channel* channel);

GOSU_FFI_API void Gosu_Channel_play(Gosu_Channel *channel);
GOSU_FFI_API bool Gosu_Channel_playing(Gosu_Channel *channel);
GOSU_FFI_API void Gosu_Channel_pause(Gosu_Channel *channel);
GOSU_FFI_API bool Gosu_Channel_paused(Gosu_Channel *channel);
GOSU_FFI_API void Gosu_Channel_resume(Gosu_Channel *channel);
GOSU_FFI_API void Gosu_Channel_stop(Gosu_Channel *channel);

GOSU_FFI_API void Gosu_Channel_set_volume(Gosu_Channel *channel, double volume);
GOSU_FFI_API void Gosu_Channel_set_speed(Gosu_Channel *channel, double speed);
GOSU_FFI_API void Gosu_Channel_set_pan(Gosu_Channel *channel, double pan);

#ifdef __cplusplus
}
#endif