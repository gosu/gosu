#include "FormattedString.hpp"
#include "GraphicsImpl.hpp"
#include <Gosu/Font.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include <array>
#include <cassert>
#include <map>
using namespace std;

struct Gosu::Font::Impl
{
    string name;
    int height;
    unsigned flags;

    // Unicode planes of 2^16 characters each. On Windows, where wchar_t is only 16 bits wide, only
    // the first plane will ever be touched.
    struct CharInfo
    {
        unique_ptr<Image> image;
        double factor;
    };
    typedef array<CharInfo, 65536> Plane;
    unique_ptr<Plane> planes[16][FF_COMBINATIONS];
    
    map<string, shared_ptr<Image>> entity_cache;
    
    CharInfo& char_info(wchar_t wc, unsigned flags)
    {
        size_t plane_index = wc / 65536;
        size_t char_index  = wc % 65536;
        
        if (plane_index >= 16) throw invalid_argument("Unicode plane out of reach");
        if (flags >= FF_COMBINATIONS) throw invalid_argument("Font flags out of range");
        
        if (!planes[plane_index][flags].get()) {
            planes[plane_index][flags].reset(new Plane);
        }
        return (*planes[plane_index][flags])[char_index];
    }
    
    const Image& image_at(const FormattedString& fs, unsigned i)
    {
        if (const char* entity = fs.entity_at(i)) {
            shared_ptr<Image>& ptr = entity_cache[entity];
            if (!ptr) {
                ptr.reset(new Image(entity_bitmap(fs.entity_at(i)), IF_SMOOTH));
            }
            return *ptr;
        }
        
        wchar_t wc     = fs.char_at(i);
        unsigned flags = fs.flags_at(i);
        CharInfo& info = char_info(wc, flags);
        
        if (info.image.get()) return *info.image;
        
        string char_string = wstring_to_utf8(wstring(1, wc));
        // TODO: Would be nice to have.
        // if (is_formatting_char(wc))
        //     char_string.clear();
        int char_width = Gosu::text_width(char_string, name, height, flags);
        
        Bitmap bitmap(char_width, height, 0x00ffffff);
        draw_text(bitmap, char_string, 0, 0, Color::WHITE, name, height, flags);
        info.image.reset(new Image(bitmap));
        info.factor = 0.5;
        return *info.image;
    }
    
    double factor_at(const FormattedString& fs, unsigned index)
    {
        if (fs.entity_at(index)) return 1;
        return char_info(fs.char_at(index), fs.flags_at(index)).factor;
    }
};

Gosu::Font::Font(int font_height, const string& font_name, unsigned font_flags)
: pimpl(new Impl)
{
    pimpl->name = font_name;
    pimpl->height = font_height * 2;
    pimpl->flags = font_flags;
}

string Gosu::Font::name() const
{
    return pimpl->name;
}

int Gosu::Font::height() const
{
    return pimpl->height / 2;
}

unsigned Gosu::Font::flags() const
{
    return pimpl->flags;
}

double Gosu::Font::text_width(const string& text, double scale_x) const
{
    wstring wtext = utf8_to_wstring(text);
    FormattedString fs(wtext.c_str(), flags());
    double result = 0;
    for (unsigned i = 0; i < fs.length(); ++i) {
        const Image& image = pimpl->image_at(fs, i);
        double factor = pimpl->factor_at(fs, i);
        result += image.width() * factor;
    }
    return result * scale_x;
}

void Gosu::Font::draw(const string& text, double x, double y, ZPos z,
    double scale_x, double scale_y, Color c, AlphaMode mode) const
{
    wstring wtext = utf8_to_wstring(text);
    FormattedString fs(wtext.c_str(), flags());
    
    for (unsigned i = 0; i < fs.length(); ++i) {
        const Image& image = pimpl->image_at(fs, i);
        double factor = pimpl->factor_at(fs, i);
        Color color = fs.entity_at(i)
                      ? Color(fs.color_at(i).alpha() * c.alpha() / 255, 255, 255, 255)
                      : multiply(fs.color_at(i), c);
        image.draw(x, y, z, scale_x * factor, scale_y * factor, color, mode);
        x += image.width() * scale_x * factor;
    }
}

void Gosu::Font::draw_rel(const string& text, double x, double y, ZPos z,
    double rel_x, double rel_y, double scale_x, double scale_y, Color c, AlphaMode mode) const
{
    x -= text_width(text) * scale_x * rel_x;
    y -= height() * scale_y * rel_y;
    
    draw(text, x, y, z, scale_x, scale_y, c, mode);
}

void Gosu::Font::set_image(wchar_t wc, const Image& image)
{
    for (unsigned flags = 0; flags < FF_COMBINATIONS; ++flags) {
        set_image(wc, flags, image);
    }
}

void Gosu::Font::set_image(wchar_t wc, unsigned font_flags, const Image& image)
{
    Impl::CharInfo& ci = pimpl->char_info(wc, font_flags);
    if (ci.image.get()) throw logic_error("Cannot set image for the same character twice");
    ci.image.reset(new Image(image));
    ci.factor = 1.0;
}

void Gosu::Font::draw_rot(const string& text, double x, double y, ZPos z, double angle,
    double scale_x, double scale_y, Color c, AlphaMode mode) const
{
    Graphics::transform(rotate(angle, x, y), [&] {
        draw(text, x, y, z, scale_x, scale_y, c, mode);
    });
}
