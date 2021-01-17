#include "Gosu_FFI_internal.h"

GOSU_FFI_API uint32_t Gosu_Color_create(uint32_t argb)
{
    return Gosu::Color{argb}.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_create_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    return Gosu::Color{a, r, g, b}.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_create_from_hsv(double h, double s, double v)
{
    return Gosu::Color::from_hsv(h, s, v).argb();
}

GOSU_FFI_API uint32_t Gosu_Color_create_from_ahsv(uint8_t alpha, double h, double s, double v)
{
    return Gosu::Color::from_ahsv(alpha, h, s, v).argb();
}

GOSU_FFI_API uint8_t Gosu_Color_alpha(uint32_t color)
{
    return Gosu::Color{color}.alpha();
}

GOSU_FFI_API uint8_t Gosu_Color_red(uint32_t color)
{
    return Gosu::Color{color}.red();
}

GOSU_FFI_API uint8_t Gosu_Color_green(uint32_t color)
{
    return Gosu::Color{color}.green();
}

GOSU_FFI_API uint8_t Gosu_Color_blue(uint32_t color)
{
    return Gosu::Color{color}.blue();
}

GOSU_FFI_API uint32_t Gosu_Color_set_alpha(uint32_t color, uint8_t value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_alpha(value);

    return gosu_color.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_set_red(uint32_t color, uint8_t value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_red(value);

    return gosu_color.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_set_green(uint32_t color, uint8_t value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_green(value);

    return gosu_color.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_set_blue(uint32_t color, uint8_t value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_blue(value);

    return gosu_color.argb();
}

GOSU_FFI_API double Gosu_Color_hue(uint32_t color)
{
    return Gosu::Color{color}.hue();
}

GOSU_FFI_API double Gosu_Color_saturation(uint32_t color)
{
    return Gosu::Color{color}.saturation();
}

GOSU_FFI_API double Gosu_Color_value(uint32_t color)
{
    return Gosu::Color{color}.value();
}

GOSU_FFI_API uint32_t Gosu_Color_set_hue(uint32_t color, double value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_hue(value);

    return gosu_color.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_set_saturation(uint32_t color, double value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_saturation(value);

    return gosu_color.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_set_value(uint32_t color, double value)
{
    Gosu::Color gosu_color{color};
    gosu_color.set_value(value);

    return gosu_color.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_argb(uint32_t color)
{
    return Gosu::Color{color}.argb();
}

GOSU_FFI_API uint32_t Gosu_Color_bgr(uint32_t color)
{
    return Gosu::Color{color}.bgr();
}

GOSU_FFI_API uint32_t Gosu_Color_abgr(uint32_t color)
{
    return Gosu::Color{color}.abgr();
}

GOSU_FFI_API uint32_t Gosu_Color_gl(uint32_t color)
{
    return color;
}
