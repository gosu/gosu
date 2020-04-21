#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char Gosu_Color_Channel;

unsigned Gosu_Color_create(unsigned argb);
unsigned Gosu_Color_create_argb(Gosu_Color_Channel a, Gosu_Color_Channel r, Gosu_Color_Channel g, Gosu_Color_Channel b);
unsigned Gosu_Color_create_from_hsv(double h, double s, double v);
unsigned Gosu_Color_create_from_ahsv(Gosu_Color_Channel alpha, double h, double s, double v);

Gosu_Color_Channel Gosu_Color_alpha(unsigned color);
Gosu_Color_Channel Gosu_Color_red(unsigned color);
Gosu_Color_Channel Gosu_Color_green(unsigned color);
Gosu_Color_Channel Gosu_Color_blue(unsigned color);
unsigned Gosu_Color_set_alpha(unsigned color, Gosu_Color_Channel value);
unsigned Gosu_Color_set_red(unsigned color, Gosu_Color_Channel value);
unsigned Gosu_Color_set_green(unsigned color, Gosu_Color_Channel value);
unsigned Gosu_Color_set_blue(unsigned color, Gosu_Color_Channel value);

double Gosu_Color_hue(unsigned color);
double Gosu_Color_saturation(unsigned color);
double Gosu_Color_value(unsigned color);
unsigned Gosu_Color_set_hue(unsigned color, double value);
unsigned Gosu_Color_set_saturation(unsigned color, double value);
unsigned Gosu_Color_set_value(unsigned color, double value);

unsigned Gosu_Color_argb(unsigned color);
unsigned Gosu_Color_bgr(unsigned color);
unsigned Gosu_Color_abgr(unsigned color);

unsigned Gosu_Color_gl(unsigned color);

#ifdef __cplusplus
}
#endif