#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>

#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <glib.h>
#include <SDL/SDL_ttf.h>

#include <string>
#include <cstring>
#include <stdexcept>

std::wstring Gosu::defaultFontName()
{
    return L"sans";
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
        PangoFontDescription *font_description;
        PangoAttribute* attr;
        PangoAttrList* attrList;

    public:
        PangoRenderer()
        {
            font_description = NULL;
            attr = NULL;
            attrList = NULL;
        }
        ~PangoRenderer()
        {
            g_object_unref(context);
            g_object_unref(layout);

            if(font_description)
                pango_font_description_free(font_description);
            if(attr)
                pango_attribute_destroy(attr);
        }
        unsigned textWidth(const std::wstring& text,
            const std::wstring& fontFace, unsigned fontHeight,
            unsigned fontFlags)
        {
            g_type_init();

            int dpi_x = 100, dpi_y = 100;

            context = pango_ft2_get_context(dpi_x, dpi_y);

            pango_context_set_language(context, pango_language_from_string ("en_US"));
            PangoDirection init_dir = PANGO_DIRECTION_LTR;
            pango_context_set_base_dir(context, init_dir);

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
        void drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
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
                    Color color = multiply(c, Gosu::Color(val, 255, 255, 255));
                    bitmap.setPixel(x2 + x, y2 + y, color);
                }
            }

            delete[] buf;
        }
    };


    // Used for custom TTF files
    // Adapted from customFont class by José Tomás Tocino García (TheOm3ga)
    class SDLTTFRenderer
    {
        SDLTTFRenderer(const SDLTTFRenderer&);
        SDLTTFRenderer& operator=(const SDLTTFRenderer&);
        
        TTF_Font* font;
        
        class SDLSurface
        {
            SDLSurface(const SDLSurface&);
            SDLSurface& operator=(const SDLSurface&);
            
            SDL_Surface* surface;
            
        public:
            SDLSurface(TTF_Font* font, const std::wstring& text, Gosu::Color c)
            {
                SDL_Color color = { c.red(), c.green(), c.blue() };
                surface = TTF_RenderUTF8_Blended(font, Gosu::wstringToUTF8(text).c_str(), color);
                if (!surface)
                    throw std::runtime_error("Could not render text " + Gosu::wstringToUTF8(text));
            }
            
            ~SDLSurface()
            {
                SDL_FreeSurface(surface);
            }
            
            unsigned height() const
            {
                return surface->h;
            }
            
            unsigned width() const
            {
                return surface->w;
            }
            
            const void* data() const
            {
                return surface->pixels;
            }
        };
        
    public:
        SDLTTFRenderer(const std::wstring& fontName, unsigned fontHeight)
        {
            static int initResult = TTF_Init();
            if (initResult < 0)
                throw std::runtime_error("Could not initialize SDL_TTF");

	          // Try to open the font at the given path
	          font = TTF_OpenFont(Gosu::wstringToUTF8(fontName).c_str(), fontHeight);
	          if (!font)
                throw std::runtime_error("Could not open TTF file " + Gosu::wstringToUTF8(fontName));
            
            // Re-open with scaled height so that ascenders/descenders fit
            int tooLargeHeight = TTF_FontHeight(font);
            int realHeight = fontHeight * fontHeight / tooLargeHeight;
            TTF_CloseFont(font);
	          font = TTF_OpenFont(Gosu::wstringToUTF8(fontName).c_str(), realHeight);
	          if (!font)
                throw std::runtime_error("Could not open TTF file " + Gosu::wstringToUTF8(fontName));
        }
        
        ~SDLTTFRenderer()
        {
            TTF_CloseFont(font);
        }

        unsigned textWidth(const std::wstring& text){
            return SDLSurface(font, text, 0xffffff).width();
        }

        void drawText(Bitmap& bmp, const std::wstring& text, int x, int y, Gosu::Color c) {
            SDLSurface surf(font, text, c);
            Gosu::Bitmap temp;
            temp.resize(surf.width(), surf.height());
            std::memcpy(temp.data(), surf.data(), temp.width() * temp.height() * 4);
            bmp.insert(temp, x, y);
        }
    };
}

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to textWidth cannot contain line breaks");
    
    if (fontName.find(L"/") == std::wstring::npos)
        return PangoRenderer().textWidth(text, fontName, fontHeight, fontFlags);
    else
        return SDLTTFRenderer(fontName, fontHeight).textWidth(text);
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to drawText cannot contain line breaks");
    
    if (fontName.find(L"/") == std::wstring::npos)
        PangoRenderer().drawText(bitmap, text, x, y, c, fontName, fontHeight, fontFlags);
    else
        SDLTTFRenderer(fontName, fontHeight).drawText(bitmap, text, x, y, c);
}
