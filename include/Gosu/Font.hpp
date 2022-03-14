#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Text.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    /// A simple bitmap font that renders and caches glyphs on demand.
    /// For large, static texts you should use Gosu::layout_text and turn the result into an image.
    class Font
    {
        struct Impl;
        std::shared_ptr<Impl> m_impl;

    public:
        /// @param name   Name of a system font, or path to a TTF file (must contain '/').
        /// @param height Height of the font, in pixels.
        /// @param font_flags  Flags used to render individual characters of the font (FontFlags enum).
        /// @param image_flags  Flags used to render individual characters of the font (ImageFlags enum).
        explicit Font(int height, const std::string& name = default_font_name(),
                      unsigned font_flags = 0, unsigned image_flags = 0);

        /// Returns the name of the font that was used to create it, i.e. the filename, nor the
        /// internal TTF name. (TODO: Why not?)
        const std::string& name() const;

        /// Returns the height of the font, in pixels.
        int height() const;

        /// Returns the flags used to render the characters of the font (FontFlags enum).
        unsigned flags() const;
        unsigned image_flags() const;

        /// Returns the width, in pixels, that the given text would occupy if drawn.
        double text_width(const std::string& text) const;
        /// Returns the width, in pixels, that the given markup would occupy if drawn.
        double markup_width(const std::string& markup) const;

        /// Draws text so the top left corner of the text is at (x; y).
        void draw_text(const std::string& text, double x, double y, ZPos z, //
                       double scale_x = 1, double scale_y = 1, Color c = Color::WHITE,
                       BlendMode mode = BM_DEFAULT) const;
        /// Draws markup so the top left corner of the text is at (x; y).
        void draw_markup(const std::string& markup, double x, double y, ZPos z, //
                         double scale_x = 1, double scale_y = 1, Color c = Color::WHITE,
                         BlendMode mode = BM_DEFAULT) const;

        /// Draws text at a position relative to (x; y).
        /// @param rel_x Determines where the text is drawn horizontally. If rel_x is 0.0, the text
        /// will be to the right of x, if it is 1.0, the text will be to the left of x, if it is
        /// 0.5, it will be centered on x.
        /// @param rel_y See rel_x.
        void draw_text_rel(const std::string& text, double x, double y, ZPos z, //
                           double rel_x, double rel_y, double scale_x = 1, double scale_y = 1,
                           Color c = Color::WHITE, BlendMode mode = BM_DEFAULT) const;
        /// Draws markup at a position relative to (x; y).
        /// @param rel_x Determines where the text is drawn horizontally. If rel_x is 0.0, the text
        /// will be to the right of x, if it is 1.0, the text will be to the left of x, if it is
        /// 0.5, it will be centered on x.
        /// @param rel_y See rel_x.
        void draw_markup_rel(const std::string& markup, double x, double y, ZPos z, //
                             double rel_x, double rel_y, double scale_x = 1, double scale_y = 1,
                             Color c = Color::WHITE, BlendMode mode = BM_DEFAULT) const;

        /// Maps a letter to a specific image, instead of generating one using Gosu's built-in text
        /// rendering.
        void set_image(std::string codepoint, unsigned font_flags, const Gosu::Image& image);
        /// A shortcut for mapping a character to an image regardless of font_flags.
        void set_image(std::string codepoint, const Gosu::Image& image);
    };
}
