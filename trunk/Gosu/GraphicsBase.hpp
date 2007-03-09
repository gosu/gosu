//! \file GraphicsBase.hpp
//! Contains general typedefs and enums related to graphics.

#ifndef GOSU_GRAPHICSBASE_HPP
#define GOSU_GRAPHICSBASE_HPP

#include <boost/cstdint.hpp>

namespace Gosu
{
    //! Represents the Z position of something drawn with Gosu's graphics
    //! system. Things with higher ZPos values will be drawn onto those with
    //! lower ZPos values. Right now, ZPos is an integer in the range of 0 to
    //! 255, but may later be extended to be an arbirary floating point value.
    typedef boost::uint8_t ZPos;

    //! Determines the way the colors are combined when one is drawn onto
    //! another.
    enum AlphaMode
    {
        //! The color's channels will be interpolated. The alpha channel
        //! specifies the opacity of the new color, 255 is full opacity.
        amDefault,
        //! The colors' channels will be added. The alpha channel specifies
        //! the percentage of the new color's channels that will be added
        //! to the old color's channels.
        amAdditive
    };

    enum FontFlags
    {
        ffBold      = 1,
        ffItalic    = 2,
        ffUnderline = 4
    };

    enum TextAlign
    {
        taLeft,
        taRight,
        taCenter,
        taJustify
    };
}

#endif
