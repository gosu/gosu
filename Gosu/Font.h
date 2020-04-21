#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Font Gosu_Font;

Gosu_Font* Gosu_Font_create(int height, const char* name, unsigned flags);

const char *Gosu_Font_name(Gosu_Font* font);
int Gosu_Font_height(Gosu_Font* font);
unsigned Gosu_Font_flags(Gosu_Font* font);

double Gosu_Font_text_width(Gosu_Font* font, const char* text);
double Gosu_Font_markup_width(Gosu_Font* font, const char* text);

void Gosu_Font_draw_text(Gosu_Font* font, const char* text, double x, double y, double z,
                          double scale_x, double scale_y, unsigned c, unsigned mode);
void Gosu_Font_draw_markup(Gosu_Font* font, const char* text, double x, double y, double z,
                            double scale_x, double scale_y, unsigned c, unsigned mode);

void Gosu_Font_draw_text_rel(Gosu_Font* font, const char* text, double x, double y, double z,
                              double rel_x, double rel_y, double scale_x, double scale_y,
                              unsigned c, unsigned mode);
void Gosu_Font_draw_markup_rel(Gosu_Font* font, const char* text, double x, double y, double z,
                                double rel_x, double rel_y, double scale_x, double scale_y,
                                unsigned c, unsigned mode);

void Gosu_Font_set_image(Gosu_Font* font, const char* codepoint, unsigned font_flags, Gosu_Image* image);

void Gosu_Font_destroy(Gosu_Font* font);

#ifdef __cplusplus
}
#endif
