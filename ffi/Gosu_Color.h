#pragma once

#include "Gosu_FFI.h"
#include "stdint.h"

GOSU_FFI_API uint32_t Gosu_Color_create(uint32_t argb);
GOSU_FFI_API uint32_t Gosu_Color_create_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
GOSU_FFI_API uint32_t Gosu_Color_create_from_hsv(double h, double s, double v);
GOSU_FFI_API uint32_t Gosu_Color_create_from_ahsv(uint8_t alpha, double h, double s, double v);

GOSU_FFI_API uint8_t Gosu_Color_alpha(uint32_t color);
GOSU_FFI_API uint8_t Gosu_Color_red(uint32_t color);
GOSU_FFI_API uint8_t Gosu_Color_green(uint32_t color);
GOSU_FFI_API uint8_t Gosu_Color_blue(uint32_t color);
GOSU_FFI_API uint32_t Gosu_Color_set_alpha(uint32_t color, uint8_t value);
GOSU_FFI_API uint32_t Gosu_Color_set_red(uint32_t color, uint8_t value);
GOSU_FFI_API uint32_t Gosu_Color_set_green(uint32_t color, uint8_t value);
GOSU_FFI_API uint32_t Gosu_Color_set_blue(uint32_t color, uint8_t value);

GOSU_FFI_API double Gosu_Color_hue(uint32_t color);
GOSU_FFI_API double Gosu_Color_saturation(uint32_t color);
GOSU_FFI_API double Gosu_Color_value(uint32_t color);
GOSU_FFI_API uint32_t Gosu_Color_set_hue(uint32_t color, double value);
GOSU_FFI_API uint32_t Gosu_Color_set_saturation(uint32_t color, double value);
GOSU_FFI_API uint32_t Gosu_Color_set_value(uint32_t color, double value);

GOSU_FFI_API uint32_t Gosu_Color_argb(uint32_t color);
GOSU_FFI_API uint32_t Gosu_Color_bgr(uint32_t color);
GOSU_FFI_API uint32_t Gosu_Color_abgr(uint32_t color);

GOSU_FFI_API uint32_t Gosu_Color_gl(uint32_t color);
