#pragma once

#include "Gosu_FFI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Song Gosu_Song;

GOSU_FFI_API Gosu_Song* Gosu_Song_create(const char* filename);
GOSU_FFI_API void Gosu_Song_destroy(Gosu_Song* song);

GOSU_FFI_API void Gosu_Song_pause(Gosu_Song* song);
GOSU_FFI_API bool Gosu_Song_paused(Gosu_Song* song);
GOSU_FFI_API void Gosu_Song_play(Gosu_Song* song, bool looping);
GOSU_FFI_API bool Gosu_Song_playing(Gosu_Song* song);
GOSU_FFI_API void Gosu_Song_stop(Gosu_Song* song);
GOSU_FFI_API double Gosu_Song_volume(Gosu_Song* song);
GOSU_FFI_API void Gosu_Song_set_volume(Gosu_Song* song, double volume);

GOSU_FFI_API Gosu_Song* Gosu_Song_current_song();

#ifdef __cplusplus
}
#endif