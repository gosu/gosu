//! \file Color.hpp
//! Interface of the Color class.

#pragma once

#include <Gosu/Platform.hpp>
#include <cstdint>

namespace Gosu
{
    //! Represents an RGBA color value with 8 bits for each channel.
    //! Can be implicitly constructed from literals of the form 0xaarrggbb.
    //! Has fast value semantics.
    //! The four-byte layout in memory is RGBA. On Big-Endian machines the unsigned int will look
    //! like 0xrrggbbaa, on Little-Endian machines it is 0xaabbggrr.
    class Color
    {
        std::uint32_t rep;
        #ifdef GOSU_IS_LITTLE_ENDIAN
        enum { RED_OFFSET = 0, GREEN_OFFSET = 8, BLUE_OFFSET = 16, ALPHA_OFFSET = 24 };
        #else
        enum { RED_OFFSET = 24, GREEN_OFFSET = 16, BLUE_OFFSET = 8, ALPHA_OFFSET = 0 };
        #endif
        
    public:
        typedef std::uint8_t Channel;
        static const unsigned GL_FORMAT = 0x1908; // GL_RGBA
        
        //! The default constructor does not initialize the color to any value.
        Color()
        {
        }
        
        //! Conversion constructor for literals of the form 0xaarrggbb.
        Color(unsigned argb)
        : Color((argb >> 24) & 0xff, (argb >> 16) & 0xff, (argb >> 8) & 0xff, (argb >> 0) & 0xff)
        {
        }
        
        Color(Channel red, Channel green, Channel blue)
        : Color(0xff, red, green, blue)
        {
        }
        
        Color(Channel alpha, Channel red, Channel green, Channel blue)
        {
            rep = (alpha << ALPHA_OFFSET) | (red << RED_OFFSET) |
                  (green << GREEN_OFFSET) | (blue << BLUE_OFFSET);
        }
        
        //! Constructs a color from the given hue/saturation/value triple.
        //! Ranges of these values are given as 0..360, 0..1, and 0..1, respectively.
        static Color from_hsv(double h, double s, double v);
        
        //! Constructs a color from the given hue/saturation/value triple.
        //! Ranges of these values are given as 0..360, 0..1, and 0..1, respectively.
        static Color from_ahsv(Channel alpha, double h, double s, double v);

        Channel red() const
        {
            return static_cast<Channel>(rep >> RED_OFFSET);
        }

        Channel green() const
        {
            return static_cast<Channel>(rep >> GREEN_OFFSET);
        }

        Channel blue() const
        {
            return static_cast<Channel>(rep >> BLUE_OFFSET);
        }

        Channel alpha() const
        {
            return static_cast<Channel>(rep >> ALPHA_OFFSET);
        }

        void set_red(Channel value)
        {
            rep &= ~(0xff << RED_OFFSET);
            rep |= value << RED_OFFSET;
        }

        void set_green(Channel value)
        {
            rep &= ~(0xff << GREEN_OFFSET);
            rep |= value << GREEN_OFFSET;
        }

        void set_blue(Channel value)
        {
            rep &= ~(0xff << BLUE_OFFSET);
            rep |= value << BLUE_OFFSET;
        }

        void set_alpha(Channel value)
        {
            rep &= ~(0xff << ALPHA_OFFSET);
            rep |= value << ALPHA_OFFSET;
        }

        //! Returns the hue of the color, in the usual range of 0..360.
        double hue() const;
        
        //! Changes the current color so hue() will return h.
        void set_hue(double h);
        
        //! Returns the saturation of the color, in the range of 0..1.
        double saturation() const;
        
        //! Changes the current color so saturation() will return s.
        void set_saturation(double s);
        
        //! Returns the value (brightness) of the color, in the range of 0..1.
        double value() const;
        
        //! Changes the current color so value() will return v.
        void set_value(double v);

        //! Returns the color in 0xaarrggbb representation.
        std::uint32_t argb() const { return alpha() << 24 | red() << 16 | green() << 8 | blue(); }

        //! Returns the color in 0x00bbggrr representation. Useful for Win32 programming.
        std::uint32_t bgr() const { return blue() << 16 | green() << 8 | red(); }

        //! Returns the color in 0xaabbggrr representation.
        std::uint32_t abgr() const { return alpha() << 24 | blue() << 16 | green() << 8 | red(); }
        
        //! Returns the internal representation of the color (RGBA in memory).
        std::uint32_t gl() const { return rep; }
        
        static const Color NONE;
        static const Color BLACK;
        static const Color GRAY;
        static const Color WHITE;
        
        static const Color AQUA;
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color YELLOW;
        static const Color FUCHSIA;
        static const Color CYAN;
    };
    
#ifndef SWIG
    inline bool operator<(Color a, Color b) { return a.gl() < b.gl(); }
    inline bool operator==(Color a, Color b) { return a.gl() == b.gl(); }
    inline bool operator!=(Color a, Color b) { return a.gl() != b.gl(); }

    //! Interpolates linearly between two colors, with a given weight towards
    //! the second color.
    //! Specialization of the general function in Gosu/Math.hpp.
    Color interpolate(Color a, Color b, double weight = 0.5);
    
    //! Combines two colors as if their channels were mapped to the 0..1 range
    //! and then multiplied with each other.
    Color multiply(Color a, Color b);
#endif
}
