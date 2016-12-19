//! \file GraphicsBase.hpp
//! Contains general typedefs and enums related to graphics.

#pragma once

#include <Gosu/Platform.hpp>
#include <array>

namespace Gosu
{
    //! Represents the Z position of something drawn with Gosu's graphics
    //! system. Draw calls with higher ZPos values will cover those with a
    //! lower ZPos value.
    typedef double ZPos;
    
    //! Determines the way colors are combined when one is drawn onto
    //! another.
    enum AlphaMode
    {
        amDefault,
        //! The color's channels will be interpolated. The alpha channel
        //! specifies the opacity of the new color, 255 is full opacity.
        amInterpolate = amDefault,
        //! The colors' channels will be added. The alpha channel specifies
        //! the percentage of the new color's channels that will be added
        //! to the old color's channels.
        amAdd,
        //! The color's channels will be multiplied with each other.
        amMultiply
    };
    
    enum FontFlags
    {
        ffBold         = 1,
        ffItalic       = 2,
        ffUnderline    = 4,
        ffCombinations = 8
    };
    
    enum TextAlign
    {
        taLeft, taRight, taCenter, taJustify
    };
    
    //! Flags that affect the tileability or interpolation of an image.
    enum ImageFlags
    {
        ifSmooth = 0,
        
        // Note: No constant for '1', but Gosu treats '1' as ifTileable for
        // backward compatibility reasons (this parameter used to be a bool).
        
        ifTileableLeft   = 1 << 1,
        ifTileableTop    = 1 << 2,
        ifTileableRight  = 1 << 3,
        ifTileableBottom = 1 << 4,
        ifTileable       = ifTileableLeft | ifTileableTop | ifTileableRight | ifTileableBottom,
        
        //! Apply nearest-neighbor interpolation when scaling this image up or
        //! down.
        ifRetro          = 1 << 5
    };
    
    typedef std::array<double, 16> Transform;
    Transform translate(double x, double y);
    Transform rotate(double angle, double aroundX = 0, double aroundY = 0);
    Transform scale(double factor);
    Transform scale(double factorX, double factorY, double fromX = 0, double fromY = 0);
    Transform concat(const Transform& lhs, const Transform& rhs);
}
