#pragma once

#include "Gosu_FFI.h"
#include "Gosu_Image.h"

typedef struct Gosu_Font Gosu_Font;

GOSU_FFI_API Gosu_Font* Gosu_Font_create(int height, const char* name, unsigned flags);
GOSU_FFI_API void Gosu_Font_destroy(Gosu_Font* font);

GOSU_FFI_API const char* Gosu_Font_name(Gosu_Font* font);
GOSU_FFI_API int Gosu_Font_height(Gosu_Font* font);
GOSU_FFI_API unsigned Gosu_Font_flags(Gosu_Font* font);

GOSU_FFI_API double Gosu_Font_text_width(Gosu_Font* font, const char* text);
GOSU_FFI_API double Gosu_Font_markup_width(Gosu_Font* font, const char* markup);

GOSU_FFI_API void Gosu_Font_draw_text(Gosu_Font* font, const char* text, double x, double y,
                                      double z, double scale_x, double scale_y, unsigned c,
                                      unsigned mode);
GOSU_FFI_API void Gosu_Font_draw_markup(Gosu_Font* font, const char* markup, double x, double y,
                                        double z, double scale_x, double scale_y, unsigned c,
                                        unsigned mode);

GOSU_FFI_API void Gosu_Font_draw_text_rel(Gosu_Font* font, const char* text, double x, double y,
                                          double z, double rel_x, double rel_y, double scale_x,
                                          double scale_y, unsigned c, unsigned mode);
GOSU_FFI_API void Gosu_Font_draw_markup_rel(Gosu_Font* font, const char* markup, double x, double y,
                                            double z, double rel_x, double rel_y, double scale_x,
                                            double scale_y, unsigned c, unsigned mode);

GOSU_FFI_API void Gosu_Font_set_image(Gosu_Font* font, const char* codepoint, unsigned font_flags,
                                      Gosu_Image* image);
