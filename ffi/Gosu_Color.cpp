#include <Gosu/Color.hpp>
#include "Gosu_Color.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned Gosu_Color_create(unsigned argb)
{
    return Gosu::Color(argb).argb();
}
unsigned Gosu_Color_create_argb(Gosu_Color_Channel a, Gosu_Color_Channel r, Gosu_Color_Channel g, Gosu_Color_Channel b)
{
    return Gosu::Color(a, r, g, b).argb();
}
unsigned Gosu_Color_create_from_hsv(double h, double s, double v)
{
    return Gosu::Color::from_hsv(h, s, v).argb();
}
unsigned Gosu_Color_create_from_ahsv(Gosu_Color_Channel alpha, double h, double s, double v)
{
    return Gosu::Color::from_ahsv(alpha, h, s, v).argb();
}

Gosu_Color_Channel Gosu_Color_alpha(unsigned color)
{
    return Gosu::Color(color).alpha();
}
Gosu_Color_Channel Gosu_Color_red(unsigned color)
{
    return Gosu::Color(color).red();
}
Gosu_Color_Channel Gosu_Color_green(unsigned color)
{
    return Gosu::Color(color).green();
}
Gosu_Color_Channel Gosu_Color_blue(unsigned color)
{
    return Gosu::Color(color).blue();
}

unsigned Gosu_Color_set_alpha(unsigned color, Gosu_Color_Channel value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_alpha(value);

    return gosu_color.argb();
}
unsigned Gosu_Color_set_red(unsigned color, Gosu_Color_Channel value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_red(value);

    return gosu_color.argb();
}
unsigned Gosu_Color_set_green(unsigned color, Gosu_Color_Channel value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_green(value);

    return gosu_color.argb();
}
unsigned Gosu_Color_set_blue(unsigned color, Gosu_Color_Channel value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_blue(value);

    return gosu_color.argb();
}

double Gosu_Color_hue(unsigned color)
{
    return Gosu::Color(color).hue();
}
double Gosu_Color_saturation(unsigned color)
{
    return Gosu::Color(color).saturation();
}
double Gosu_Color_value(unsigned color)
{
    return Gosu::Color(color).value();
}

unsigned Gosu_Color_set_hue(unsigned color, double value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_hue(value);

    return gosu_color.argb();
}
unsigned Gosu_Color_set_saturation(unsigned color, double value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_saturation(value);

    return gosu_color.argb();
}
unsigned Gosu_Color_set_value(unsigned color, double value)
{
    Gosu::Color gosu_color = Gosu::Color(color);
    gosu_color.set_value(value);

    return gosu_color.argb();
}

unsigned Gosu_Color_argb(unsigned color)
{
    return Gosu::Color(color).argb();
}
unsigned Gosu_Color_bgr(unsigned color)
{
    return Gosu::Color(color).bgr();
}
unsigned Gosu_Color_abgr(unsigned color)
{
    return Gosu::Color(color).abgr();
}

unsigned Gosu_Color_gl(unsigned color)
{
    return color;
}

#ifdef __cplusplus
}
#endif