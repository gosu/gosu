#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>

#include <SDL_ttf.h>
#include <glib.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>

#include <cstring>
#include <stdexcept>
#include <string>

std::string Gosu::default_font_name()
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
        
        int text_width(const std::string& text, const std::string& font_face,
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
        
        void draw_text(Bitmap& bitmap, const std::string& text, int x, int y, Color c,
            const std::string& font_face, int font_height, unsigned font_flags)
        {
            text_width(text, font_face, font_height, font_flags);

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

            int min_height = std::min<int>(height, font_height);

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

    // Used for custom TTF files
    // Adapted from custom_font class by José Tomás Tocino García (TheOm3ga)
    class SDLTTFRenderer
    {
        SDLTTFRenderer(const SDLTTFRenderer&) = delete;
        SDLTTFRenderer& operator=(const SDLTTFRenderer&) = delete;
        SDLTTFRenderer(SDLTTFRenderer&&) = delete;
        SDLTTFRenderer& operator=(SDLTTFRenderer&&) = delete;
        
        TTF_Font* font;
        
        class SDLSurface
        {
            SDLSurface(const SDLSurface&);
            SDLSurface& operator=(const SDLSurface&);
            
            SDL_Surface* surface;
            
        public:
            SDLSurface(TTF_Font* font, const std::string& text, Gosu::Color c)
            {
                // This is intentionally re-ordered to BGR. This way, the surface pixels do not
                // have to be converted from RGB to BGR later in the process.
                SDL_Color color = { c.blue(), c.green(), c.red() };
                surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
                if (!surface) {
                    throw std::runtime_error("Could not render text: " + text);
                }
            }
            
            ~SDLSurface()
            {
                SDL_FreeSurface(surface);
            }
            
            int height() const
            {
                return surface->h;
            }
            
            int width() const
            {
                return surface->w;
            }
            
            const void* data() const
            {
                return surface->pixels;
            }
        };
        
    public:
        SDLTTFRenderer(const std::string& font_name, int font_height)
        {
            static int init_result = TTF_Init();
            if (init_result < 0) {
                throw std::runtime_error("Could not initialize SDL_TTF");
            }

            // Try to open the font at the given path
            font = TTF_OpenFont(font_name.c_str(), font_height);
            if (!font) {
                throw std::runtime_error("Could not open TTF file " + font_name);
            }
            
            // Re-open with scaled height so that ascenders/descenders fit
            int too_large_height = TTF_FontHeight(font);
            int real_height = font_height * font_height / too_large_height;
            TTF_CloseFont(font);
            font = TTF_OpenFont(font_name.c_str(), real_height);
            if (!font) {
                throw std::runtime_error("Could not open TTF file " + font_name);
            }
        }
        
        ~SDLTTFRenderer()
        {
            TTF_CloseFont(font);
        }

        int text_width(const std::string& text)
        {
            return SDLSurface(font, text, 0xffffff).width();
        }

        void draw_text(Bitmap& bmp, const std::string& text, int x, int y, Gosu::Color c)
        {
            SDLSurface surf(font, text, c);
            Gosu::Bitmap temp;
            temp.resize(surf.width(), surf.height());
            std::memcpy(temp.data(), surf.data(), temp.width() * temp.height() * 4);
            bmp.insert(temp, x, y);
        }
    };
}

int Gosu::text_width(const std::string& text, const std::string& font_name,
                     int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw std::invalid_argument("the argument to text_width cannot contain line breaks");
    }
    
    if (font_name.find("/") == font_name.npos) {
        return PangoRenderer().text_width(text, font_name, font_height, font_flags);
    }
    else {
        return SDLTTFRenderer(font_name, font_height).text_width(text);
    }
}

void Gosu::draw_text(Bitmap& bitmap, const std::string& text, int x, int y, Color c,
    const std::string& font_name, int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw std::invalid_argument("the argument to draw_text cannot contain line breaks");
    }
    
    if (font_name.find("/") == font_name.npos) {
        PangoRenderer().draw_text(bitmap, text, x, y, c, font_name, font_height, font_flags);
    }
    else {
        SDLTTFRenderer(font_name, font_height).draw_text(bitmap, text, x, y, c);
    }
}

#endif
