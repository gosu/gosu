#pragma once

#include "Gosu_Channel.h"
#include "Gosu_Color.h"
#include "Gosu_FFI.h"
#include "Gosu_Font.h"
#include "Gosu_Image.h"
#include "Gosu_Sample.h"
#include "Gosu_Song.h"
#include "Gosu_TextInput.h"
#include "Gosu_Window.h"
#include <stdint.h>

// Error reporting
GOSU_FFI_API const char* Gosu_last_error(void);

// Graphics operations
GOSU_FFI_API void Gosu_gl_z(double z, void function(void* data), void* data);
GOSU_FFI_API void Gosu_gl(void function(void* data), void* data);
GOSU_FFI_API void Gosu_flush(void);
GOSU_FFI_API Gosu_Image* Gosu_render(int width, int height, void function(void* data), void* data,
                                     unsigned image_flags);
GOSU_FFI_API Gosu_Image* Gosu_record(int width, int height, void function(void* data), void* data);
GOSU_FFI_API void Gosu_clip_to(double x, double y, double width, double height,
                               void function(void* data), void* data);

// Transformations
GOSU_FFI_API void Gosu_transform(double m0, double m1, double m2, double m3, double m4, double m5,
                                 double m6, double m7, double m8, double m9, double m10, double m11,
                                 double m12, double m13, double m14, double m15,
                                 void function(void* data), void* data);
GOSU_FFI_API void Gosu_translate(double x, double y, void function(void* data), void* data);
GOSU_FFI_API void Gosu_scale(double scale_x, double scale_y, double around_x, double around_y,
                             void function(void* data), void* data);
GOSU_FFI_API void Gosu_rotate(double angle, double around_x, double around_y,
                              void function(void* data), void* data);

// Rendering
GOSU_FFI_API void Gosu_draw_line(double x1, double y1, unsigned c1, double x2, double y2,
                                 unsigned c2, double z, unsigned mode);
GOSU_FFI_API void Gosu_draw_triangle(double x1, double y1, unsigned c1, double x2, double y2,
                                     unsigned c2, double x3, double y3, unsigned c3, double z,
                                     unsigned mode);
GOSU_FFI_API void Gosu_draw_rect(double x, double y, double width, double height, unsigned c,
                                 double z, unsigned mode);
GOSU_FFI_API void Gosu_draw_quad(double x1, double y1, unsigned c1, double x2, double y2,
                                 unsigned c2, double x3, double y3, unsigned c3, double x4,
                                 double y4, unsigned c4, double z, unsigned mode);

// Math functions
GOSU_FFI_API double Gosu_distance(double x1, double y1, double x2, double y2);

GOSU_FFI_API double Gosu_angle(double from_x, double from_y, double to_x, double to_y);
GOSU_FFI_API double Gosu_angle_diff(double from, double to);

GOSU_FFI_API double Gosu_offset_x(double angle, double radius);
GOSU_FFI_API double Gosu_offset_y(double angle, double radius);

GOSU_FFI_API double Gosu_random(double min, double max);

// Window/Screen information
GOSU_FFI_API int Gosu_available_width(Gosu_Window* window);
GOSU_FFI_API int Gosu_available_height(Gosu_Window* window);
GOSU_FFI_API int Gosu_screen_width(Gosu_Window* window);
GOSU_FFI_API int Gosu_screen_height(Gosu_Window* window);
GOSU_FFI_API double Gosu_dpi(Gosu_Window* window);

// Button querying
GOSU_FFI_API int Gosu_button_down(int id);
GOSU_FFI_API const char* Gosu_button_id_to_char(int id);
GOSU_FFI_API unsigned Gosu_button_char_to_id(const char* character);
GOSU_FFI_API const char* Gosu_button_name(int id);
GOSU_FFI_API const char* Gosu_gamepad_name(int id);
GOSU_FFI_API double Gosu_axis(int id);

// Misc
GOSU_FFI_API int Gosu_fps(void);
GOSU_FFI_API void Gosu_user_languages(void function(void* data, const char* language),
                                      void* data);
GOSU_FFI_API uint64_t Gosu_milliseconds(void);
GOSU_FFI_API const char* Gosu_default_font_name(void);
