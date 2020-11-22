#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <Gosu/Window.h>
#include <Gosu/Image.h>
#include <Gosu/Font.h>
#include <Gosu/Color.h>
#include <Gosu/TextInput.h>
#include <Gosu/Sample.h>
#include <Gosu/Song.h>
#include <Gosu/Channel.h>

// Graphics operations
void Gosu_gl_z(double z, void function(void *data), void* data);
void Gosu_gl(void function(void *data), void* data);
void Gosu_flush();
Gosu_Image* Gosu_render(int width, int height, void function(void *data), void* data, unsigned image_flags);
Gosu_Image* Gosu_record(int width, int height, void function(void *data), void* data);
void Gosu_clip_to(double x, double y, double width, double height, void function(void *data), void* data);

// Transformations
void Gosu_transform(double m0, double m1, double m2, double m3, double m4, double m5, double m6,
                    double m7, double m8, double m9, double m10, double m11, double m12, double m13,
                    double m14, double m15, void function(void *data), void* data);
void Gosu_translate(double x, double y, void function(void *data), void* data);
void Gosu_scale(double scale_x, double scale_y, double around_x, double around_y, void function(void *data), void* data);
void Gosu_rotate(double angle, double around_x, double around_y, void function(void *data), void* data);

void Gosu_clip_to(double x, double y, double width, double height, void function(void *data), void* data);

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

double Gosu_offset_x(double angle, double radius);
double Gosu_offset_y(double angle, double radius);

double Gosu_random(double min, double max);

// Window/Screen information
unsigned Gosu_available_width(Gosu_Window* window);
unsigned Gosu_available_height(Gosu_Window* window);
unsigned Gosu_screen_width(Gosu_Window* window);
unsigned Gosu_screen_height(Gosu_Window* window);

// Button querying
int Gosu_button_down(int id);
const char* Gosu_button_id_to_char(int id);
unsigned Gosu_button_char_to_id(const char* character);
const char *Gosu_button_name(int id);
const char *Gosu_gamepad_name(int id);
double Gosu_axis(int id);

// Misc
int Gosu_fps();
const char* Gosu_language();
long Gosu_milliseconds();
const char* Gosu_default_font_name();

#ifdef __cplusplus
}
#endif