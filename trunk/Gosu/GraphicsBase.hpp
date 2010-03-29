//! \file GraphicsBase.hpp
//! Contains general typedefs and enums related to graphics.

#ifndef GOSU_GRAPHICSBASE_HPP
#define GOSU_GRAPHICSBASE_HPP

#include <limits>

namespace Gosu
{
    //! Represents the Z position of something drawn with Gosu's graphics
    //! system. Draw calls with higher ZPos values will cover those with a
    //! lower ZPos value, that is, they are performed last.
    typedef double ZPos;
    
    //! The lowest possible Z position. By using this, you tell Gosu that
    //! your drawing operation does not need Z ordering and can be performed
    //! immediately.
    const double zImmediate = -std::numeric_limits<double>::infinity();
    
    //! Determines the way colors are combined when one is drawn onto
    //! another.
    enum AlphaMode
    {
        //! The color's channels will be interpolated. The alpha channel
        //! specifies the opacity of the new color, 255 is full opacity.
        amDefault,
        //! The colors' channels will be added. The alpha channel specifies
        //! the percentage of the new color's channels that will be added
        //! to the old color's channels.
        amAdditive,
        //! The color's channels will be multiplied with each other.
        amMultiply
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
    
    //! Flags that affect the tileability of an image.
    enum BorderFlags
    {
        bfSmooth = 0,
        bfTileableLeft = 1,
        bfTileableTop = 2,
        bfTileableRight = 4,
        bfTileableBottom = 8,
        bfTileable = bfTileableLeft | bfTileableTop | bfTileableRight | bfTileableBottom
    };        
}

#endif
