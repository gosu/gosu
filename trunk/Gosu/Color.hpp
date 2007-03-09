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
    };

    inline bool operator==(Color a, Color b)
    {
        return a.argb() == b.argb();
    }

    inline bool operator!=(Color a, Color b)
    {
        return a.argb() != b.argb();
    }

    Color interpolate(Color a, Color b);
    Color multiply(Color a, Color b);
    
    struct HSV
    {
        // h, s: range [0..1]
        // v: range [0..360)
        double h, s, v;
    };
    
    HSV colorToHSV(const Color& c);
    Color hsvToColor(const HSV& hsv);    

    //! Namespace which contains some predefined colors.
    namespace Colors
    {
        const Color none    = 0x00000000;
        const Color black   = 0xff000000;
        const Color gray    = 0xff808080;
        const Color white   = 0xffffffff;
        const Color red     = 0xffff0000;
        const Color green   = 0xff00ff00;
        const Color blue    = 0xff0000ff;
        const Color yellow  = 0xffffff00;
        const Color fuchsia = 0xffff00ff;
    }
}

#endif
