#ifdef GOSU_ENABLE_VIDEO

#pragma once

#include "Gosu_FFI.h"

typedef struct Gosu_Video Gosu_Video;

GOSU_FFI_API Gosu_Video* Gosu_Video_create(const char* filename, unsigned image_flags);
GOSU_FFI_API Gosu_Video* Gosu_Video_create_scaled(const char* filename, int width, int height, unsigned image_flags);
GOSU_FFI_API void Gosu_Video_destroy(Gosu_Video* video);

GOSU_FFI_API unsigned Gosu_Video_width(Gosu_Video* video);
GOSU_FFI_API unsigned Gosu_Video_height(Gosu_Video* video);

GOSU_FFI_API void Gosu_Video_draw(Gosu_Video* video, double x, double y, double z, double scale_x,
                                  double scale_y, unsigned color, unsigned mode);
GOSU_FFI_API void Gosu_Video_draw_rot(Gosu_Video* video, double x, double y, double z, double angle,
                                      double center_x, double center_y, double scale_x,
                                      double scale_y, unsigned color, unsigned mode);
GOSU_FFI_API void Gosu_Video_draw_as_quad(Gosu_Video* video, double x1, double y1, unsigned color1,
                                          double x2, double y2, unsigned color2, double x3,
                                          double y3, unsigned color3, double x4, double y4,
                                          unsigned color4, double z, unsigned mode);

GOSU_FFI_API void Gosu_Video_pause(Gosu_Video* video);
GOSU_FFI_API bool Gosu_Video_paused(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_play(Gosu_Video* video, bool looping);
GOSU_FFI_API bool Gosu_Video_playing(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_stop(Gosu_Video* video);
GOSU_FFI_API double Gosu_Video_volume(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_set_volume(Gosu_Video* video, double volume);
GOSU_FFI_API double Gosu_Video_length(Gosu_Video* video);
GOSU_FFI_API double Gosu_Video_position(Gosu_Video* video);
GOSU_FFI_API void Gosu_Video_set_position(Gosu_Video* video, double position);

#endif