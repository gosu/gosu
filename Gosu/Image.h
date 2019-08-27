#pragma once

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct Gosu_Image Gosu_Image;

typedef struct Gosu_GLTexInfo {
  int texture_name;
  double left, right, top, bottom;
} Gosu_GLTexInfo;

// Constructor
Gosu_Image* Gosu_Image_create(const char *filename, unsigned image_flags);
Gosu_Image* Gosu_Image_create_from_blob(const char *source, int width, int height, unsigned image_flags); // support for RMagick blob objects
Gosu_Image* Gosu_Image_create_from_markup(const char *markup, unsigned image_flags);
Gosu_Image* Gosu_Image_create_from_text(const char *text, unsigned image_flags);

// TODO: make this return an array
Gosu_Image* Gosu_Image_create_from_tiles(Gosu_Image *source, int tile_width, int tile_height, unsigned image_flags);
Gosu_Image* Gosu_Image_create_from_subimage(Gosu_Image *source, int left, int top, int width, int height);

// Destructor
void Gosu_Image_destroy(Gosu_Image *image);

// Image properties
unsigned Gosu_Image_width(Gosu_Image *image);
unsigned Gosu_Image_height(Gosu_Image *image);

// Rendering
void Gosu_Image_draw(Gosu_Image *image, double x, double y, double z, unsigned color, unsigned mode);
void Gosu_Image_draw_rot(Gosu_Image *image, double x, double y, double z,
                          double angle, double center_x, double center_y,
                          double scale_x, double scale_y, unsigned color, unsigned mode);
void Gosu_Image_draw_as_quad(Gosu_Image* image, double x1, double y1, unsigned color1,
                               double x2, double y2, unsigned color2,
                               double x3, double y3, unsigned color3,
                               double x4, double y4, unsigned color4,
                               double z, unsigned mode);

// Operations
void Gosu_Image_insert(Gosu_Image *image, Gosu_Image *source, int x, int y);
void Gosu_Image_save(Gosu_Image *image, const char *filename);
const char* Gosu_Image_to_blob(Gosu_Image *image);
Gosu_GLTexInfo* Gosu_Image_gl_texinfo(Gosu_Image *image);

#ifdef __cplusplus
  }
#endif