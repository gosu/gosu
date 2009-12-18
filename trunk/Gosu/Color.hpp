//! \file Color.hpp
//! Interface of the Color class.

#ifndef GOSU_COLOR_HPP
#define GOSU_COLOR_HPP

#include <boost/cstdint.hpp>

namespace Gosu
{
    //! Represents an ARGB color value with 8 bits for each channel. Can be
    //! implicitly constructed from literals of the form 0xaarrggbb. Has fast
    //! value semantics.
    class Color
    {
        boost::uint32_t rep;

    public:
        typedef boost::uint8_t Channel;

        //! The default constructor does not initialize the color to any value.
        Color()
        {
        }

        //! Conversion constructor for literals of the form 0xaarrggbb.
        //! (C++ only.)
        Color(boost::uint32_t argb)
        : rep(argb)
        {
        }

        Color(Channel red, Channel green, Channel blue)
        {
            rep = (0xff << 24) | (red << 16) | (green << 8) | blue;
        }

        Color(Channel alpha, Channel red, Channel green, Channel blue)
        {
            rep = (alpha << 24) | (red << 16) | (green << 8) | blue;
        }
        
        //! Constructs a color from the given hue/saturation/value triple.
        //! Ranges of these values are given as 0..360, 0..1 and 0..1,
        //! respectively.
        //! The alpha value is set to 1 from this method.
        static Color fromHSV(double h, double s, double v);
        static Color fromAHSV(Channel alpha, double h, double s, double v);

        Channel alpha() const
        {
            return static_cast<Channel>((rep >> 24) & 0xff);
        }

        Channel red() const
        {
            return static_cast<Channel>((rep >> 16) & 0xff);
        }

        Channel green() const
        {
            return static_cast<Channel>((rep >> 8) & 0xff);
        }

        Channel blue() const
        {
            return static_cast<Channel>(rep & 0xff);
        }

        void setAlpha(Channel value)
        {
            rep &= 0x00ffffff;
            rep |= value << 24;
        }

        void setRed(Channel value)
        {
            rep &= 0xff00ffff;
            rep |= value << 16;
        }

        void setGreen(Channel value)
        {
            rep &= 0xffff00ff;
            rep |= value << 8;
        }

        void setBlue(Channel value)
        {
            rep &= 0xffffff00;
            rep |= value;
        }

        //! Returns the hue of the color, in the usual range of 0..360.
        double hue() const;
        
        //! Changes the current color so hue() will return h.
        void setHue(double h);
        
        //! Returns the saturation of the color, in the range of 0..1.
        double saturation() const;
        
        //! Changes the current color so saturation() will return s.
        void setSaturation(double s);
        
        //! Returns the value (brightness) of the color, in the range of 0..1.
        double value() const;
        
        //! Changes the current color so value() will return v.
        void setValue(double v);

        //! Returns the color in 0xaarrggbb representation.
        boost::uint32_t argb() const
        {
            return rep;
        }

        //! Returns the color in 0x00bbggrr representation.
        boost::uint32_t bgr() const
        {
            return red() | (rep & 0x0000ff00) | blue() << 16;
        }

        //! Returns the color in 0xaabbggrr representation.
        boost::uint32_t abgr() const
        {
            return alpha() << 24 | bgr();
        }
        
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
    
    // Causes weird errors when included in the SWIG wrapping process.
    // If, with a future version of SWIG, this can be included and
    // require 'gosu'; include Gosu
    // works from within Ruby, the #ifndef guard can be removed.
    #ifndef SWIG
    inline bool operator==(Color a, Color b)
    {
        return a.argb() == b.argb();
    }

    inline bool operator!=(Color a, Color b)
    {
        return a.argb() != b.argb();
    }
    #endif

    //! Interpolates linearly between two colors, with a given weight towards
    //! the second color.
    //! Specialization of the general function in Gosu/Math.hpp.
    Color interpolate(Color a, Color b, double weight = 0.5);
    
    //! Combines two colors as if their channels were mapped to the 0..1 range
    //! and then multiplied with each other.
    Color multiply(Color a, Color b);

    // Deprecated
    namespace Colors
    {
        const Color none    = 0x00000000;
        const Color black   = 0xff000000;
        const Color gray    = 0xff808080;
        const Color white   = 0xffffffff;
        
        const Color aqua    = 0xff00ffff;
        const Color red     = 0xffff0000;
        const Color green   = 0xff00ff00;
        const Color blue    = 0xff0000ff;
        const Color yellow  = 0xffffff00;
        const Color fuchsia = 0xffff00ff;
        const Color cyan    = 0xff00ffff;
    }
}

#endif
