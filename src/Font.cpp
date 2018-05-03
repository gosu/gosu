#include "MarkupParser.hpp"
#include "GraphicsImpl.hpp"
#include <Gosu/Font.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include "utf8proc.h"
#include <array>
#include <cassert>
#include <map>
using namespace std;

static const int FONT_RENDER_SCALE = 2;

struct Gosu::Font::Impl
{
    string name;
    int height;
    unsigned base_flags;

    // The most common characters are stored directly in an array for maximum performance.
    // (It probably makes sense to just store the full Basic Multilingual Plane in here.)
    array<array<Image, 128>, FF_COMBINATIONS> ascii_graphemes;
    // Everything else is looked up through a map...
    array<map<string, Image>, FF_COMBINATIONS> other_graphemes;
    
    const Image& image(const string& grapheme, unsigned font_flags)
    {
        Image* image;
        if (grapheme.length() == 1 && grapheme[0] < ascii_graphemes[font_flags].size()) {
            image = &ascii_graphemes[font_flags][grapheme[0]];
        }
        else {
            image = &other_graphemes[font_flags][grapheme];
        }
        
        // If this grapheme has not been rendered before, do it now.
        if (image->height() == 0) {
            *image = Image(create_text(grapheme, name, height * FONT_RENDER_SCALE, font_flags));
        }
        
        return *image;
    }
    
    template<typename Functor>
    void for_each_grapheme(const FormattedString& fstr, const Functor& functor)
    {
        utf8proc_int32_t grapheme_state = 0;
        auto* ptr = (utf8proc_uint8_t*) fstr.string.c_str();
        auto* end_of_string = ptr + fstr.string.length();
        auto* begin_of_grapheme = ptr;
        utf8proc_uint32_t last_cp = 0;
        
        auto flush_current_grapheme = [&] {
            if (begin_of_grapheme != ptr) {
                functor(image(string(begin_of_grapheme, ptr), fstr.flags));
                begin_of_grapheme = ptr;
            }
        };
        
        while (ptr < end_of_string) {
            utf8proc_int32_t codepoint;
            auto len = utf8proc_iterate(ptr, end_of_string - ptr, &codepoint);
            // Cancel parsing when invalid UTF-8 is encountered.
            if (len < 1) break;
            
            if (last_cp && utf8proc_grapheme_break_stateful(last_cp, codepoint, &grapheme_state)) {
                flush_current_grapheme();
            }

            last_cp = codepoint;
            ptr += len;
        }
        
        flush_current_grapheme();
    }
};

Gosu::Font::Font(int font_height, const string& font_name, unsigned font_flags)
: pimpl(new Impl)
{
    pimpl->name = font_name;
    pimpl->height = font_height;
    pimpl->base_flags = font_flags;
}

const string& Gosu::Font::name() const
{
    return pimpl->name;
}

int Gosu::Font::height() const
{
    return pimpl->height;
}

unsigned Gosu::Font::flags() const
{
    return pimpl->base_flags;
}

double Gosu::Font::text_width(const string& text, double scale_x) const
{
    int width = 0;

    // Split the text into lines (split_words = false) because Font doesn't implement word-wrapping.
    MarkupParser(text.c_str(), pimpl->base_flags, false, [&](vector<FormattedString>&& line) {
        int line_width = 0;
        for (auto& part : line) {
            pimpl->for_each_grapheme(part, [&](const Image& image) {
                line_width += image.width();
            });
        }
        width = max(width, line_width);
    }).parse();
    
    return width / FONT_RENDER_SCALE;
}

void Gosu::Font::draw(const string& text, double x, double y, ZPos z,
    double scale_x, double scale_y, Color c, AlphaMode mode) const
{
    double current_y = y;
    
    // Split the text into lines (split_words = false) because Font doesn't implement word-wrapping.
    MarkupParser(text.c_str(), pimpl->base_flags, false, [&](vector<FormattedString>&& line) {
        double current_x = x;
        for (auto& part : line) {
            pimpl->for_each_grapheme(part, [&](const Image& image) {
                image.draw(current_x, current_y, z,
                           scale_x / FONT_RENDER_SCALE, scale_y / FONT_RENDER_SCALE,
                           c, mode);
                current_x += scale_x * image.width() / FONT_RENDER_SCALE;
            });
        }
        current_y += scale_y * height();
    }).parse();
}

void Gosu::Font::draw_rel(const string& text, double x, double y, ZPos z,
    double rel_x, double rel_y, double scale_x, double scale_y, Color c, AlphaMode mode) const
{
    x -= text_width(text) * scale_x * rel_x;
    y -= height() * scale_y * rel_y;
    
    draw(text, x, y, z, scale_x, scale_y, c, mode);
}

void Gosu::Font::draw_rot(const string& text, double x, double y, ZPos z, double angle,
    double scale_x, double scale_y, Color c, AlphaMode mode) const
{
    Graphics::transform(rotate(angle, x, y), [&] {
        draw(text, x, y, z, scale_x, scale_y, c, mode);
    });
}

