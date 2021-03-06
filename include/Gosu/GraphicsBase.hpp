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
    enum BlendMode
    {
        BM_DEFAULT,
        //! The color's channels will be interpolated. The alpha channel
        //! specifies the opacity of the new color, 255 is full opacity.
        BM_INTERPOLATE = BM_DEFAULT,
        //! The colors' channels will be added. The alpha channel specifies
        //! the percentage of the new color's channels that will be added
        //! to the old color's channels.
        BM_ADD,
        //! The color's channels will be multiplied with each other.
        BM_MULTIPLY
    };
    
    enum FontFlags
    {
        FF_BOLD         = 1,
        FF_ITALIC       = 2,
        FF_UNDERLINE    = 4,
        FF_COMBINATIONS = 8
    };
    
    enum Alignment
    {
        AL_LEFT,
        AL_RIGHT,
        AL_CENTER,
        AL_JUSTIFY
    };
    
    //! Flags that affect the tileability or interpolation of an image.
    enum ImageFlags
    {
        IF_SMOOTH = 0,
        
        // Note: No constant for '1', but Gosu treats '1' as IF_TILEABLE for
        // backward compatibility reasons (this parameter used to be a bool).
        
        IF_TILEABLE_LEFT   = 1 << 1,
        IF_TILEABLE_TOP    = 1 << 2,
        IF_TILEABLE_RIGHT  = 1 << 3,
        IF_TILEABLE_BOTTOM = 1 << 4,
        IF_TILEABLE        = IF_TILEABLE_LEFT | IF_TILEABLE_TOP | IF_TILEABLE_RIGHT | IF_TILEABLE_BOTTOM,
        
        //! Apply nearest-neighbor interpolation when scaling this image up or
        //! down.
        IF_RETRO           = 1 << 5
    };
    
    typedef std::array<double, 16> Transform;
    Transform translate(double x, double y);
    Transform rotate(double angle, double around_x = 0, double around_y = 0);
    Transform scale(double factor);
    Transform scale(double scale_x, double scale_y, double from_x = 0, double from_y = 0);
    Transform concat(const Transform& lhs, const Transform& rhs);
}
