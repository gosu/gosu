#include <Gosu/Gosu.hpp>

extern "C" {
#include "Gosu_Image.h"
#include "Gosu_Font.h"

Gosu_Font *Gosu_Font_create(int height, const char *name, unsigned flags)
{
    return reinterpret_cast<Gosu_Font *>(new Gosu::Font(height, name, flags));
}

const char *Gosu_Font_name(Gosu_Font *font)
{
    return reinterpret_cast<Gosu::Font *>(font)->name().c_str();
}

int Gosu_Font_height(Gosu_Font *font)
{
    return reinterpret_cast<Gosu::Font *>(font)->height();
}

unsigned Gosu_Font_flags(Gosu_Font *font)
{
    return reinterpret_cast<Gosu::Font *>(font)->flags();
}

double Gosu_Font_text_width(Gosu_Font *font, const char *text)
{
    return reinterpret_cast<Gosu::Font *>(font)->text_width(text);
}

double Gosu_Font_markup_width(Gosu_Font *font, const char *text)
{
    return reinterpret_cast<Gosu::Font *>(font)->markup_width(text);
}

void Gosu_Font_draw_text(Gosu_Font *font, const char *text, double x, double y, double z,
                            double scale_x, double scale_y, unsigned c, unsigned mode)
{
    reinterpret_cast<Gosu::Font *>(font)->draw_text(text, x, y, z, scale_x, scale_y, c, (Gosu::AlphaMode)mode);
}

void Gosu_Font_draw_markup(Gosu_Font *font, const char *text, double x, double y, double z,
                            double scale_x, double scale_y, unsigned c, unsigned mode)
{
    reinterpret_cast<Gosu::Font *>(font)->draw_markup(text, x, y, z, scale_x, scale_y, c, (Gosu::AlphaMode)mode);
}

void Gosu_Font_draw_text_rel(Gosu_Font *font, const char *text, double x, double y, double z,
                                double rel_x, double rel_y, double scale_x, double scale_y,
                                unsigned c, unsigned mode)
{
    reinterpret_cast<Gosu::Font *>(font)->draw_text_rel(text, x, y, z, rel_x, rel_y, scale_x, scale_y, c, (Gosu::AlphaMode)mode);
}

void Gosu_Font_draw_markup_rel(Gosu_Font *font, const char *text, double x, double y, double z,
                                double rel_x, double rel_y, double scale_x, double scale_y,
                                unsigned c, unsigned mode)
{
    reinterpret_cast<Gosu::Font *>(font)->draw_markup_rel(text, x, y, z, rel_x, rel_y, scale_x, scale_y, c, (Gosu::AlphaMode)mode);
}

void Gosu_Font_set_image(Gosu_Font *font, const char *codepoint, unsigned font_flags, Gosu_Image *image)
{

    const Gosu::Image *gosu_image = reinterpret_cast<Gosu::Image *>(image);
    reinterpret_cast<Gosu::Font *>(font)->set_image(codepoint, font_flags, *gosu_image);
}

void Gosu_Font_destroy(Gosu_Font *font)
{
    delete (reinterpret_cast<Gosu::Font *>(font));
}
}