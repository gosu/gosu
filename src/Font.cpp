#include <Gosu/Font.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include "GraphicsImpl.hpp"
#include "MarkupParser.hpp"
#include <cmath> // for std::ceil
#include <mutex>
#include <stdexcept>
#include <unordered_map>

struct Gosu::Font::Impl : private Gosu::Noncopyable
{
    const int height;
    const std::string name;
    const unsigned base_flags;
    const unsigned image_flags;

    Impl(int height, std::string_view name, unsigned base_flags, unsigned image_flags)
        : height(height),
          name(name),
          base_flags(base_flags),
          image_flags(image_flags)
    {
    }

    struct GlyphKey
    {
        char32_t codepoint;
        unsigned font_flags;
        bool operator==(const GlyphKey&) const = default;
    };

    struct GlyphKeyHasher
    {
        std::size_t operator()(const GlyphKey& key) const noexcept
        {
            // The highest legal Unicode code point is 0x10FFFF, which is a 21-bit value, so we do
            // not have to worry about overwriting any of the higher bits.
            // This only works if unordered_map uses prime table sizes (which it should):
            // https://www.reddit.com/r/cpp_questions/comments/us3nyb/comment/i937ulb/
            return key.codepoint | (key.font_flags << 24);
        }
    };

    // Font implements copying through its shared_ptr, not by making a true copy. However, Fonts are
    // not immutable: Glyphs are created and cashed on demand, and set_image() enables modifications
    // to the shared data of a font. Having multiple references to the same object, but not being
    // able to use it from two threads, seems counterintuitive, so introduce a mutex here, even
    // though most Gosu games/programs will never really require it.
    // (Could be a shared_mutex, but doesn't seem to be worth the trouble.)
    std::mutex glyphs_mutex;
    std::unordered_map<GlyphKey, Image, GlyphKeyHasher> glyphs;

    const Image& image(char32_t codepoint, unsigned font_flags)
    {
        const GlyphKey key { codepoint, font_flags };
        if (const auto iterator = glyphs.find(key); iterator != glyphs.end()) {
            return iterator->second;
        }

        // If this codepoint has not been rendered before, do it now.
        // By default, render each glyph at 200% its size so that we have some wiggle room for
        // changing the font size dynamically without it appearing too blurry.
        auto scaled_height = height * 2;
        // Optimization: Don't render higher-resolution versions if we use
        // next neighbor interpolation anyway.
        if (image_flags & IF_RETRO) {
            scaled_height = height;
        }

        std::u32string string(1, codepoint);
        Bitmap bitmap(scaled_height, scaled_height);
        const int required_width = static_cast<int>(std::ceil(
            Gosu::draw_text(bitmap, 0, 0, Color::WHITE, string, name, scaled_height, font_flags)));
        if (required_width > bitmap.width()) {
            // If the character was wider than high, we need to render it again.
            Bitmap resized_bitmap(required_width, scaled_height);
            std::swap(resized_bitmap, bitmap);
            Gosu::draw_text(bitmap, 0, 0, Color::WHITE, string, name, scaled_height, font_flags);
        }
        const Rect source_rect { 0, 0, required_width, bitmap.height() };

        return glyphs[key] = Image(bitmap, source_rect, image_flags);
    }
};

Gosu::Font::Font(int font_height, std::string_view font_name, unsigned font_flags,
                 unsigned image_flags)
    : m_impl(new Impl(font_height, font_name, font_flags, image_flags))
{
}

const std::string& Gosu::Font::name() const
{
    return m_impl->name;
}

int Gosu::Font::height() const
{
    return m_impl->height;
}

unsigned Gosu::Font::flags() const
{
    return m_impl->base_flags;
}

unsigned Gosu::Font::image_flags() const
{
    return m_impl->image_flags;
}

double Gosu::Font::text_width(const std::string& text) const
{
    return markup_width(escape_markup(text));
}

double Gosu::Font::markup_width(const std::string& markup) const
{
    const std::unique_lock lock(m_impl->glyphs_mutex);

    double width = 0;

    // Split the text into lines (split_words = false) because Font doesn't implement word-wrapping.
    MarkupParser parser(m_impl->base_flags, false, [&](const std::vector<FormattedString>& line) {
        double line_width = 0;
        for (const auto& part : line) {
            for (const auto codepoint : part.text) {
                const auto& image = m_impl->image(codepoint, part.flags);
                double image_scale = image.height() ? 1.0 * height() / image.height() : 1.0;
                line_width += image_scale * image.width();
            }
        }
        width = std::max(width, line_width);
    });
    parser.parse(markup);

    return width;
}

void Gosu::Font::draw_text(const std::string& text, double x, double y, ZPos z, //
                           double scale_x, double scale_y, Color c, BlendMode mode) const
{
    draw_markup(escape_markup(text), x, y, z, scale_x, scale_y, c, mode);
}

void Gosu::Font::draw_markup(const std::string& markup, double x, double y, ZPos z, //
                             double scale_x, double scale_y, Color c, BlendMode mode) const
{
    const std::unique_lock lock(m_impl->glyphs_mutex);

    double current_y = y;

    // Split the text into lines (split_words = false) because Font doesn't implement word-wrapping.
    MarkupParser parser(m_impl->base_flags, false, [&](const std::vector<FormattedString>& line) {
        double current_x = x;
        for (const auto& part : line) {
            for (const auto codepoint : part.text) {
                const auto& image = m_impl->image(codepoint, part.flags);
                double image_scale = image.height() ? 1.0 * height() / image.height() : 1.0;
                image.draw(current_x, current_y, z, image_scale * scale_x, image_scale * scale_y,
                           multiply(c, part.color), mode);
                current_x += image_scale * scale_x * image.width();
            }
        }
        current_y += scale_y * height();
    });
    parser.parse(markup);
}

void Gosu::Font::draw_text_rel(const std::string& text, double x, double y, ZPos z, //
                               double rel_x, double rel_y, double scale_x, double scale_y, //
                               Color c, BlendMode mode) const
{
    if (rel_x != 0) {
        x -= text_width(text) * scale_x * rel_x;
    }
    if (rel_y != 0) {
        y -= height() * scale_y * rel_y;
    }

    draw_text(text, x, y, z, scale_x, scale_y, c, mode);
}

void Gosu::Font::draw_markup_rel(const std::string& markup, double x, double y, ZPos z, //
                                 double rel_x, double rel_y, double scale_x, double scale_y,
                                 Color c, BlendMode mode) const
{
    if (rel_x != 0) {
        x -= markup_width(markup) * scale_x * rel_x;
    }
    if (rel_y != 0) {
        y -= height() * scale_y * rel_y;
    }

    draw_markup(markup, x, y, z, scale_x, scale_y, c, mode);
}

void Gosu::Font::set_image(std::string_view codepoint, unsigned font_flags,
                           const Gosu::Image& image)
{
    const std::u32string utc4 = utf8_to_composed_utc4(codepoint);
    if (utc4.length() != 1) {
        throw std::invalid_argument("Could not compose '" + std::string(codepoint)
                                    + "' into single codepoint");
    }

    const std::unique_lock lock(m_impl->glyphs_mutex);
    m_impl->glyphs.insert_or_assign({ .codepoint = utc4[0], .font_flags = font_flags }, image);
}

void Gosu::Font::set_image(std::string_view codepoint, const Gosu::Image& image)
{
    for (unsigned font_flags = 0; font_flags < FF_COMBINATIONS; ++font_flags) {
        set_image(codepoint, font_flags, image);
    }
}
