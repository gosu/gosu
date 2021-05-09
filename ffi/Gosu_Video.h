#ifdef ENABLE_FFMPEG

#pragma once

#include "Gosu_FFI.h"

typedef struct Gosu_Video Gosu_Video;

GOSU_FFI_API Gosu_Video* Gosu_Video_create(const char* filename, unsigned image_flags);
GOSU_FFI_API Gosu_Video* Gosu_Video_create_scaled(const char* filename, int width, int height, unsigned image_flags);
GOSU_FFI_API void Gosu_Video_destroy(Gosu_Video* video);

GOSU_FFI_API void Gosu_Video_pause(Gosu_Video* video);
GOSU_FFI_API bool Gosu_Video_paused(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_play(Gosu_Video* video, bool looping);
GOSU_FFI_API bool Gosu_Video_playing(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_stop(Gosu_Video* video);
GOSU_FFI_API double Gosu_Video_volume(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_set_volume(Gosu_Video* video, double volume);

#endif