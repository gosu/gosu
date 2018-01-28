#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>

#include "TextImpl.hpp"

#include <glib.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>

#include <cstring>
#include <stdexcept>
#include <string>

using namespace std;


string Gosu::default_font_name()
{
    return "sans";
}

namespace Gosu
{
    // Used for system fonts
    // Adapted from original version by Jan Lücker
    class PangoRenderer
    {
        PangoRenderer(const PangoRenderer&);
        PangoRenderer& operator=(const PangoRenderer&);
        
        int width, height;

        PangoContext* context;
        PangoLayout* layout;
        PangoFontDescription* font_description;
        PangoAttribute* attr;
        PangoAttrList* attr_list;

    public:
        PangoRenderer()
        {
            font_description = NULL;
            attr = NULL;
            attr_list = NULL;
        }
        
        ~PangoRenderer()
        {
            g_object_unref(context);
            g_object_unref(layout);

            if (font_description) {
                pango_font_description_free(font_description);
            }
            
            if (attr) {
                pango_attribute_destroy(attr);
            }
        }
        
        int text_width(const string& text, const string& font_face,
                       int font_height, unsigned font_flags)
        {
            g_type_init();

            int dpi_x = 100, dpi_y = 100;

            context = pango_ft2_get_context(dpi_x, dpi_y);

            pango_context_set_language(context, pango_language_from_string("en_US"));
            PangoDirection init_dir = PANGO_DIRECTION_LTR;
            pango_context_set_base_dir(context, init_dir);

            font_description = pango_font_description_new();

            pango_font_description_set_family(font_description, g_strdup(font_face.c_str()));
            pango_font_description_set_style(font_description,
                (font_flags & FF_ITALIC) ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
            pango_font_description_set_variant(font_description, PANGO_VARIANT_NORMAL);
            pango_font_description_set_weight(font_description,
                (font_flags & FF_BOLD) ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
            pango_font_description_set_stretch(font_description, PANGO_STRETCH_NORMAL);
            int init_scale = int(font_height / 2.0 + 0.5);
            pango_font_description_set_size(font_description, init_scale * PANGO_SCALE);

            pango_context_set_font_description(context, font_description);

            layout = pango_layout_new(context);

            if (font_flags & FF_UNDERLINE) {
                attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
                attr->start_index = 0;
                attr->end_index = text.length();
                attr_list = pango_attr_list_new();
                pango_attr_list_insert(attr_list, attr);
                pango_layout_set_attributes(layout, attr_list);
                pango_attr_list_unref(attr_list);
            }

            pango_layout_set_text(layout, text.c_str(), -1);

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
        
        void draw_text(Bitmap& bitmap, const string& text, int x, int y, Color c,
            const string& font_face, int font_height, unsigned font_flags)
        {
            text_width(text, font_face, font_height, font_flags);

            FT_Bitmap ft_bitmap;

            guchar* buf = new guchar[width * height];
            fill(buf, buf + width * height, 0x00);

            ft_bitmap.rows = height;
            ft_bitmap.width = width;
            ft_bitmap.pitch = ft_bitmap.width;
            ft_bitmap.buffer = buf;
            ft_bitmap.num_grays = 256;
            ft_bitmap.pixel_mode = ft_pixel_mode_grays;

            int x_start = 0;
            pango_ft2_render_layout(&ft_bitmap, layout, x_start, 0);

            int min_height = min<int>(height, font_height);

            for (int y2 = 0; y2 < min_height; y2++) {
                if (y + y2 < 0 || y + y2 >= bitmap.height()) break;
                
                for (int x2 = 0; x2 < width; x2++) {
                    if (x + x2 < 0 || x + x2 >= bitmap.width()) break;
                    unsigned val = ft_bitmap.buffer[y2 * width + x2];
                    Color color = multiply(c, Gosu::Color(val, 255, 255, 255));
                    bitmap.set_pixel(x2 + x, y2 + y, color);
                }
            }

            delete[] buf;
        }
    };
}

int Gosu::text_width(const string& text, const string& font_name,
                     int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("the argument to text_width cannot contain line breaks");
    }
    
    if (font_name.find_first_of("./\\") != text.npos) {
        return text_width_ttf(text, font_name, font_height, font_flags);
    }

    return PangoRenderer().text_width(text, font_name, font_height, font_flags);
}

void Gosu::draw_text(Bitmap& bitmap, const string& text, int x, int y, Color c,
                     const string& font_name, int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("the argument to draw_text cannot contain line breaks");
    }

    if (font_name.find_first_of("./\\") != text.npos) {
        return draw_text_ttf(bitmap, text, x, y, c, font_name, font_height, font_flags);
    }
    
    PangoRenderer().draw_text(bitmap, text, x, y, c, font_name, font_height, font_flags);
}

#endif
