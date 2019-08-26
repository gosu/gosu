#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Color Gosu_Color;
typedef unsigned char Channel;

Gosu_Color* Gosu_Color_create(unsigned argb);
Gosu_Color* Gosu_Color_create_argb(Channel a, Channel r, Channel g, Channel b);
Gosu_Color* Gosu_Color_create_from_hsv(double h, double s, double v);
Gosu_Color* Gosu_Color_create_from_ahsv(Channel alpha, double h, double s, double v);

Channel Gosu_Color_alpha(Gosu_Color* color);
Channel Gosu_Color_red(Gosu_Color* color);
Channel Gosu_Color_green(Gosu_Color* color);
Channel Gosu_Color_blue(Gosu_Color* color);
void Gosu_Color_set_alpha(Gosu_Color* color, Channel value);
void Gosu_Color_set_red(Gosu_Color* color, Channel value);
void Gosu_Color_set_green(Gosu_Color* color, Channel value);
void Gosu_Color_set_blue(Gosu_Color* color, Channel value);

double Gosu_Color_hue(Gosu_Color* color);
double Gosu_Color_saturation(Gosu_Color* color);
double Gosu_Color_value(Gosu_Color* color);
void Gosu_Color_set_hue(Gosu_Color* color, double value);
void Gosu_Color_set_saturation(Gosu_Color* color, double value);
void Gosu_Color_set_value(Gosu_Color* color, double value);

unsigned Gosu_Color_argb(Gosu_Color* color);
unsigned Gosu_Color_bgr(Gosu_Color* color);
unsigned Gosu_Color_abgr(Gosu_Color* color);

unsigned Gosu_Color_gl(Gosu_Color* color);

void Gosu_Color_destroy(Gosu_Color* color);

#ifdef __cplusplus
}
#endif