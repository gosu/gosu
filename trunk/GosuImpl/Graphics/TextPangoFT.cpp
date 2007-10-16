#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>

#include <pango/pango.h>
#include <pango/pangoft2.h>

#include <glib.h>

#include <string>

std::wstring Gosu::defaultFontName()
{
    return L"sans";
}

namespace Gosu
{
    class pango
    {
        int width, height;

        PangoContext* context;
        PangoLayout* layout;
        /*static*/ PangoFontDescription *font_description;
        PangoAttribute* attr;
        PangoAttrList* attrList;

    public:
        pango();
        ~pango();
        unsigned textWidth(const std::wstring& text,
            const std::wstring& fontFace, unsigned fontHeight,
            unsigned fontFlags);
        void drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
            Color c, const std::wstring& fontFace, unsigned fontHeight,
            unsigned fontFlags);
    };
}

Gosu::pango::pango()
{
    font_description = NULL;
    attr = NULL;
    attrList = NULL;
}

Gosu::pango::~pango()
{
    g_object_unref(context);
    g_object_unref(layout);
    
    if(font_description)
        pango_font_description_free(font_description);
    if(attr)
        pango_attribute_destroy(attr);
}

unsigned Gosu::pango::textWidth(const std::wstring& text,
    const std::wstring& fontFace, unsigned fontHeight,
    unsigned fontFlags)
{
    g_type_init();

    int dpi_x = 100, dpi_y = 100;

    context = pango_ft2_get_context(dpi_x, dpi_y);

    pango_context_set_language(context, pango_language_from_string ("en_US"));
    PangoDirection init_dir = PANGO_DIRECTION_LTR;
    pango_context_set_base_dir(context, init_dir);

//    static PangoFontDescription *font_description;
    font_description = pango_font_description_new();

    pango_font_description_set_family(font_description,
        g_strdup(narrow(fontFace).c_str()));
    pango_font_description_set_style(font_description,
        (fontFlags & ffItalic) ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
    pango_font_description_set_variant(font_description, PANGO_VARIANT_NORMAL);
    pango_font_description_set_weight(font_description,
        (fontFlags & ffBold) ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
    pango_font_description_set_stretch(font_description, PANGO_STRETCH_NORMAL);
    int init_scale = int(fontHeight/2.0 + 0.5);
    pango_font_description_set_size(font_description, init_scale * PANGO_SCALE);

    pango_context_set_font_description(context, font_description);


    layout = pango_layout_new(context);


    if(fontFlags & ffUnderline)
    {
//        PangoAttribute *attr;
        attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
        attr->start_index = 0;
        attr->end_index = text.length();
//        PangoAttrList* attrList;
        attrList = pango_attr_list_new();
        pango_attr_list_insert(attrList, attr);
        pango_layout_set_attributes(layout, attrList);
        pango_attr_list_unref(attrList);
    }


    // IMPR: Catch errors? (Last NULL-Pointer)
    gchar* utf8Str = g_ucs4_to_utf8((gunichar*)text.c_str(), text.length(), NULL, NULL, NULL);
    pango_layout_set_text(layout, utf8Str, -1);
    g_free(utf8Str);

    PangoDirection base_dir = pango_context_get_base_dir(context);
    pango_layout_set_alignment(layout,
        base_dir == PANGO_DIRECTION_LTR ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT);

    pango_layout_set_width(layout, -1);

    PangoRectangle logical_rect;

    pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
    height = logical_rect.height;
    width = logical_rect.width;

    return width;
}

void Gosu::pango::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontFace, unsigned fontHeight,
    unsigned fontFlags)
{
    textWidth(text, fontFace, fontHeight, fontFlags);

    FT_Bitmap ft_bitmap;

    guchar* buf = new guchar[width * height];
    std::fill(buf, buf + width * height, 0x00);

    ft_bitmap.rows = height;
    ft_bitmap.width = width;
    ft_bitmap.pitch = ft_bitmap.width;
    ft_bitmap.buffer = buf;
    ft_bitmap.num_grays = 256;
    ft_bitmap.pixel_mode = ft_pixel_mode_grays;

    int x_start = 0;
    pango_ft2_render_layout(&ft_bitmap, layout, x_start, 0);


    int min_height = height;
    if((unsigned)height > fontHeight) min_height = fontHeight;

    for(int y2 = 0; y2 < min_height; y2++)
    {
        if (y + y2 < 0 || y + y2 >= bitmap.height())
            break;
        
        for(int x2 = 0; x2 < width; x2++)
        {
            if (x + x2 < 0 || x + x2 >= bitmap.width())
                break;
            unsigned val = ft_bitmap.buffer[y2*width+x2];
            Color color = c;
            color.setAlpha(val);
            bitmap.setPixel(x2 + x, y2 + y, color);
        }
    }

    delete[] buf;
}


unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontFace, unsigned fontHeight,
    unsigned fontFlags)
{
    pango helper;
    return helper.textWidth(text, fontFace, fontHeight, fontFlags);
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontFace, unsigned fontHeight,
    unsigned fontFlags)
{
    pango helper;
    helper.drawText(bitmap, text, x, y, c, fontFace, fontHeight, fontFlags);
}
