#pragma once

#include "Gosu_FFI.h"
#include <stddef.h> // for size_t
#include <stdint.h>

typedef struct Gosu_Image Gosu_Image;

typedef struct Gosu_GLTexInfo
{
    uint32_t tex_name;
    double left, right, top, bottom;
} Gosu_GLTexInfo;

// Constructor
GOSU_FFI_API Gosu_Image* Gosu_Image_create(const char* filename, unsigned image_flags);
GOSU_FFI_API Gosu_Image* Gosu_Image_create_rect(const char* filename, //
                                                int x, int y, int width, int height,
                                                unsigned image_flags);
GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_blob(void* blob, size_t byte_count, //
                                                     int columns, int rows, //
                                                     int x, int y, int width, int height,
                                                     unsigned image_flags);
GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_markup(const char* markup, const char* font,
                                                       double font_height, int width,
                                                       double spacing, unsigned align,
                                                       unsigned font_flags, unsigned image_flags);
GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_text(const char* text, const char* font,
                                                     double font_height, int width, double spacing,
                                                     unsigned align, unsigned font_flags,
                                                     unsigned image_flags);
GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_subimage(Gosu_Image* source, int left, int top,
                                                         int width, int height);

GOSU_FFI_API void Gosu_Image_create_from_tiles(const char* source, int tile_width, int tile_height,
                                               void function(void* data, Gosu_Image* image),
                                               void* data, unsigned image_flags);
GOSU_FFI_API void Gosu_Image_create_tiles_from_image(Gosu_Image* image, int tile_width,
                                                     int tile_height,
                                                     void function(void* data, Gosu_Image* image),
                                                     void* data, unsigned image_flags);

// Destructor
GOSU_FFI_API void Gosu_Image_destroy(Gosu_Image* image);

// Image properties
GOSU_FFI_API unsigned Gosu_Image_width(Gosu_Image* image);
GOSU_FFI_API unsigned Gosu_Image_height(Gosu_Image* image);

// Rendering
GOSU_FFI_API void Gosu_Image_draw(Gosu_Image* image, double x, double y, double z, double scale_x,
                                  double scale_y, unsigned color, unsigned mode);
GOSU_FFI_API void Gosu_Image_draw_rot(Gosu_Image* image, double x, double y, double z, double angle,
                                      double center_x, double center_y, double scale_x,
                                      double scale_y, unsigned color, unsigned mode);
GOSU_FFI_API void Gosu_Image_draw_as_quad(Gosu_Image* image, double x1, double y1, unsigned color1,
                                          double x2, double y2, unsigned color2, double x3,
                                          double y3, unsigned color3, double x4, double y4,
                                          unsigned color4, double z, unsigned mode);

// Operations
GOSU_FFI_API void Gosu_Image_insert(Gosu_Image* image, Gosu_Image* source, int x, int y);
GOSU_FFI_API void Gosu_Image_save(Gosu_Image* image, const char* filename);
GOSU_FFI_API uint8_t* Gosu_Image_to_blob(Gosu_Image* image);
GOSU_FFI_API Gosu_GLTexInfo* Gosu_Image_gl_tex_info_create(Gosu_Image* image);
GOSU_FFI_API void Gosu_Image_gl_tex_info_destroy(Gosu_GLTexInfo* tex_info);
