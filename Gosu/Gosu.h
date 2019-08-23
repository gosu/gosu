#pragma once

#ifdef __cplusplus
  extern "C" {
#endif

#include <Gosu/Window.h>
#include <Gosu/Image.h>
#include <Gosu/Font.h>
// #include <Gosu/TextInput.h>
// #include <Gosu/Sample.h>
// #include <Gosu/Song.h>
// #include <Gosu/Channel.h>
// #include <Gosu/Color.h>
// #include <Gosu/GLTexInfo.h>

// #include "Gosu/Buttons.h"

// Graphics operations
void Gosu_gl(void function());
void Gosu_flush();
Gosu_Image* Gosu_render(int width, int height, void function(), unsigned image_flags);
Gosu_Image* Gosu_record(int width, int height, void function());
void Gosu_clip_to(double x, double y, double width, double height, void function());

// Transformations
void Gosu_transform();
void Gosu_translate(double x, double y, void function());
void Gosu_scale(double scale_x, double scale_y, double around_x, double around_y, void function());
void Gosu_rotate(double angle, double around_x, double around_y, void function());

// Rendering
void Gosu_draw_line(double x1, double y1, unsigned c1,
                    double x2, double y2, unsigned c2,
                    double z, unsigned mode);
void Gosu_draw_triangle(double x1, double y1, unsigned c1,
                        double x2, double y2, unsigned c2,
                        double x3, double y3, unsigned c3,
                        double z, unsigned mode);
void Gosu_draw_rect(double x, double y, double width, double height, unsigned c, double z, unsigned mode);
void Gosu_draw_quad(double x1, double y1, unsigned c1,
                    double x2, double y2, unsigned c2,
                    double x3, double y3, unsigned c3,
                    double x4, double y4, unsigned c4,
                    double z, unsigned mode);

// Math functions
double Gosu_distance(double x1, double y1, double x2, double y2);

double Gosu_angle(double from_x, double from_y, double to_x, double to_y);
double Gosu_angle_diff(double from, double to);

double Gosu_offset_x(double theta, double r);
double Gosu_offset_y(double theta, double r);

double Gosu_random(double min, double max);

// Window/Screen information
int Gosu_available_width();
int Gosu_available_height();
int Gosu_screen_width();
int Gosu_screen_height();

// Button querying
int Gosu_button_down(int id);
const char* Gosu_button_id_to_char(int id);
unsigned Gosu_button_char_to_id(const char* character);

// Misc
int Gosu_fps();
const char* Gosu_language();
long Gosu_milliseconds();
const char* Gosu_default_font_name();

#ifdef __cplusplus
  }
#endif