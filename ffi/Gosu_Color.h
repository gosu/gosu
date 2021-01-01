#pragma once

#include "Gosu_FFI.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char Gosu_Color_Channel;

GOSU_FFI_API unsigned Gosu_Color_create(unsigned argb);
GOSU_FFI_API unsigned Gosu_Color_create_argb(Gosu_Color_Channel a, Gosu_Color_Channel r, Gosu_Color_Channel g, Gosu_Color_Channel b);
GOSU_FFI_API unsigned Gosu_Color_create_from_hsv(double h, double s, double v);
GOSU_FFI_API unsigned Gosu_Color_create_from_ahsv(Gosu_Color_Channel alpha, double h, double s, double v);

GOSU_FFI_API Gosu_Color_Channel Gosu_Color_alpha(unsigned color);
GOSU_FFI_API Gosu_Color_Channel Gosu_Color_red(unsigned color);
GOSU_FFI_API Gosu_Color_Channel Gosu_Color_green(unsigned color);
GOSU_FFI_API Gosu_Color_Channel Gosu_Color_blue(unsigned color);
GOSU_FFI_API unsigned Gosu_Color_set_alpha(unsigned color, Gosu_Color_Channel value);
GOSU_FFI_API unsigned Gosu_Color_set_red(unsigned color, Gosu_Color_Channel value);
GOSU_FFI_API unsigned Gosu_Color_set_green(unsigned color, Gosu_Color_Channel value);
GOSU_FFI_API unsigned Gosu_Color_set_blue(unsigned color, Gosu_Color_Channel value);

GOSU_FFI_API double Gosu_Color_hue(unsigned color);
GOSU_FFI_API double Gosu_Color_saturation(unsigned color);
GOSU_FFI_API double Gosu_Color_value(unsigned color);
GOSU_FFI_API unsigned Gosu_Color_set_hue(unsigned color, double value);
GOSU_FFI_API unsigned Gosu_Color_set_saturation(unsigned color, double value);
GOSU_FFI_API unsigned Gosu_Color_set_value(unsigned color, double value);

GOSU_FFI_API unsigned Gosu_Color_argb(unsigned color);
GOSU_FFI_API unsigned Gosu_Color_bgr(unsigned color);
GOSU_FFI_API unsigned Gosu_Color_abgr(unsigned color);

GOSU_FFI_API unsigned Gosu_Color_gl(unsigned color);

#ifdef __cplusplus
}
#endif