#include "Gosu_FFI_internal.h"

GOSU_FFI_API Gosu_Font* Gosu_Font_create(int height, const char* name, unsigned flags, unsigned image_flags)
{
    return Gosu_translate_exceptions([=] {
        return new Gosu_Font{Gosu::Font{height, name, flags, image_flags}};
    });
}

GOSU_FFI_API void Gosu_Font_destroy(Gosu_Font* font)
{
    delete font;
}

GOSU_FFI_API const char* Gosu_Font_name(Gosu_Font* font)
{
    static thread_local std::string name;

    return Gosu_translate_exceptions([=] {
        name = font->font.name();
        return name.c_str();
    });
}

GOSU_FFI_API int Gosu_Font_height(Gosu_Font* font)
{
    return Gosu_translate_exceptions([=] {
        return font->font.height();
    });
}

GOSU_FFI_API double Gosu_Font_text_width(Gosu_Font* font, const char* text)
{
    return Gosu_translate_exceptions([=] {
        return font->font.text_width(text);
    });
}

GOSU_FFI_API double Gosu_Font_markup_width(Gosu_Font* font, const char* markup)
{
    return Gosu_translate_exceptions([=] {
        return font->font.markup_width(markup);
    });
}

GOSU_FFI_API void Gosu_Font_draw_text(Gosu_Font* font, const char* text, double x, double y,
                                      double z, double scale_x, double scale_y, unsigned c,
                                      unsigned mode)
{
    Gosu_translate_exceptions([=] {
        font->font.draw_text(text, x, y, z, scale_x, scale_y, c,
                             static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Font_draw_markup(Gosu_Font* font, const char* markup, double x, double y,
                                        double z, double scale_x, double scale_y, unsigned c,
                                        unsigned mode)
{
    Gosu_translate_exceptions([=] {
        font->font.draw_markup(markup, x, y, z, scale_x, scale_y, c,
                               static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Font_draw_text_rel(Gosu_Font* font, const char* text, double x, double y,
                                          double z, double rel_x, double rel_y, double scale_x,
                                          double scale_y, unsigned c, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        font->font.draw_text_rel(text, x, y, z, rel_x, rel_y, scale_x, scale_y, c,
                                 static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Font_draw_markup_rel(Gosu_Font* font, const char* markup, double x, double y,
                                            double z, double rel_x, double rel_y, double scale_x,
                                            double scale_y, unsigned c, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        font->font.draw_markup_rel(markup, x, y, z, rel_x, rel_y, scale_x, scale_y, c,
                                   static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Font_set_image(Gosu_Font* font, const char* codepoint, unsigned font_flags,
                                      Gosu_Image* image)
{
    Gosu_translate_exceptions([=] {
        font->font.set_image(codepoint, font_flags, image->image);
    });
}
