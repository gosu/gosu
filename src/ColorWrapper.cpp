#include <Gosu/Color.hpp>
#include <Gosu/Color.h>

#ifdef __cplusplus
extern "C" {
#endif

Gosu_Color* Gosu_Color_create(unsigned argb)
{
  return reinterpret_cast<Gosu_Color*>( new Gosu::Color(argb) );
}
Gosu_Color* Gosu_Color_create_argb(Channel a, Channel r, Channel g, Channel b)
{
  return reinterpret_cast<Gosu_Color*>( new Gosu::Color(a, r, g, b) );
}
Gosu_Color* Gosu_Color_create_from_hsv(double h, double s, double v)
{
  return reinterpret_cast<Gosu_Color*>( &Gosu::Color::from_hsv(h, s, v) );
}
Gosu_Color* Gosu_Color_create_from_ahsv(Channel alpha, double h, double s, double v)
{
  return reinterpret_cast<Gosu_Color*>( &Gosu::Color::from_ahsv(alpha, h, s, v) );
}

Channel Gosu_Color_alpha(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->alpha();
}
Channel Gosu_Color_red(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->red();
}
Channel Gosu_Color_green(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->green();
}
Channel Gosu_Color_blue(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->blue();
}

void Gosu_Color_set_alpha(Gosu_Color* color, Channel value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_alpha(value);
}
void Gosu_Color_set_red(Gosu_Color* color, Channel value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_red(value);
}
void Gosu_Color_set_green(Gosu_Color* color, Channel value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_green(value);
}
void Gosu_Color_set_blue(Gosu_Color* color, Channel value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_blue(value);
}

double Gosu_Color_hue(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->hue();
}
double Gosu_Color_saturation(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->saturation();
}
double Gosu_Color_value(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->value();
}

void Gosu_Color_set_hue(Gosu_Color* color, double value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_hue(value);
}
void Gosu_Color_set_saturation(Gosu_Color* color, double value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_saturation(value);
}
void Gosu_Color_set_value(Gosu_Color* color, double value)
{
  return reinterpret_cast<Gosu::Color*>( color )->set_value(value);
}

unsigned Gosu_Color_argb(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->argb();
}
unsigned Gosu_Color_bgr(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->bgr();
}
unsigned Gosu_Color_abgr(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->abgr();
}

unsigned Gosu_Color_gl(Gosu_Color* color)
{
  return reinterpret_cast<Gosu::Color*>( color )->gl();
}

void Gosu_Color_destroy(Gosu_Color* color)
{
  delete( reinterpret_cast<Gosu::Color*>( color ) );
}

#ifdef __cplusplus
}
#endif