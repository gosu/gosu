//! \file Font.hpp
//! Interface of the Font class.

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
    //! Fonts are ideal for drawing short, dynamic strings.
    //! For large, static texts you should use Gosu::create_text and turn the result into
    //! an image.
    class Font
    {
        struct Impl;
        std::shared_ptr<Impl> pimpl;

    public:
        //! Constructs a font that can be drawn onto the graphics object.
        //! \param font_name Name of a system font, or a filename to a TTF
        //!        file (must contain '/', does not work on Linux).
        //! \param font_height Height of the font, in pixels.
        //! \param font_flags Flags used to render individual characters of
        //!        the font.
        Font(int height, const std::string& name = default_font_name(), unsigned flags = FF_BOLD);
        
        //! Returns the name of the font that was used to create it.
        const std::string& name() const;
        
        //! Returns the height of the font, in pixels.
        int height() const;
        
        //! Returns the flags used to create the font characters.
        unsigned flags() const;
        
        //! Returns the width, in pixels, the given text would occupy if drawn.
        double text_width(const std::string& text, double scale_x = 1) const;
        
        //! Draws text so the top left corner of the text is at (x; y).
        //! \param text Formatted text without line-breaks.
        void draw(const std::string& text, double x, double y, ZPos z,
                  double scale_x = 1, double scale_y = 1, Color c = Color::WHITE,
                  AlphaMode mode = AM_DEFAULT) const;
        
        //! Draws text at a position relative to (x; y).
        //! \param rel_x Determines where the text is drawn horizontally. If
        //! rel_x is 0.0, the text will be to the right of x, if it is 1.0,
        //! the text will be to the left of x, if it is 0.5, it will be
        //! centered on x. Of course, all real numbers are possible values.
        //! \param rel_y See rel_x.
        void draw_rel(const std::string& text, double x, double y, ZPos z,
                      double rel_x, double rel_y, double scale_x = 1, double scale_y = 1,
                      Color c = Color::WHITE, AlphaMode mode = AM_DEFAULT) const;

        //! DEPRECATED: Analogous to draw, but rotates the text by a given angle.
        //! Use Graphics::push_transform to achieve the same effect.
    #ifndef SWIG
        GOSU_DEPRECATED
    #endif
        void draw_rot(const std::string& text, double x, double y, ZPos z, double angle,
                      double scale_x = 1, double scale_y = 1,
                      Color c = Color::WHITE, AlphaMode mode = AM_DEFAULT) const;

        //! Maps a letter to a specific image instead of generating one using
        //! Gosu's built-in text rendering.
        void set_image(std::string codepoint, unsigned font_flags, const Gosu::Image& image);
        //! A shortcut for mapping a character to an image regardless of font_flags.
        void set_image(std::string codepoint, const Gosu::Image& image);
    };
}
