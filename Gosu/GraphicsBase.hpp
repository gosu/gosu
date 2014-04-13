//! \file GraphicsBase.hpp
//! Contains general typedefs and enums related to graphics.

#ifndef GOSU_GRAPHICSBASE_HPP
#define GOSU_GRAPHICSBASE_HPP

#include <Gosu/Platform.hpp>
#include <Gosu/TR1.hpp>
#include <limits>

namespace Gosu
{
    //! Represents the Z position of something drawn with Gosu's graphics
    //! system. Draw calls with higher ZPos values will cover those with a
    //! lower ZPos value, that is, they are performed last.
    typedef double ZPos;
    
    //! Determines the way colors are combined when one is drawn onto
    //! another.
#if defined(GOSU_CPP11_ENABLED)
    enum class AlphaMode
    {
        //! The color's channels will be interpolated. The alpha channel
        //! specifies the opacity of the new color, 255 is full opacity.
        DEFAULT,
        INTERPOLATE = DEFAULT,
        //! The colors' channels will be added. The alpha channel specifies
        //! the percentage of the new color's channels that will be added
        //! to the old color's channels.
        ADD,
        //! The color's channels will be multiplied with each other.
        MULTIPLY
    };
    GOSU_DEPRECATED constexpr AlphaMode amDefault = AlphaMode::DEFAULT;
    GOSU_DEPRECATED constexpr AlphaMode amInterpolate = AlphaMode::INTERPOLATE;
    GOSU_DEPRECATED constexpr AlphaMode amAdd = AlphaMode::ADD;
    GOSU_DEPRECATED constexpr AlphaMode amAdditive = AlphaMode::ADD;
    GOSU_DEPRECATED constexpr AlphaMode amMultiply = AlphaMode::MULTIPLY;
#else
    enum AlphaMode
    {
        //! The color's channels will be interpolated. The alpha channel
        //! specifies the opacity of the new color, 255 is full opacity.
        amDefault,
        //! The colors' channels will be added. The alpha channel specifies
        //! the percentage of the new color's channels that will be added
        //! to the old color's channels.
        amAdd,
        amAdditive = amAdd,
        //! The color's channels will be multiplied with each other.
        amMultiply
    };
#endif
    
    enum FontFlags
    {
        ffBold         = 1,
        ffItalic       = 2,
        ffUnderline    = 4,
        ffCombinations = 8
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
    
#ifdef GOSU_IS_MAC
    // TODO: Without this gigantic hack, Gosu crashes in the "scale" function,
    // but _only_ when used from Ruby 1.9. It is unclear what might cause this -
    // maybe a compiler bug that tries to use SSE functions with the wrong
    // alignment. Adding __attribute__((aligned(16))) does not help, though.
    struct Transform
    {
        double value[16];
        bool operator==(const Transform &other) { for (int i = 0; i < 16; ++i) if ((*this)[i] != other[i]) return false; return true; }
        const double &operator[](std::size_t idx) const { return value[idx]; }
        double &operator[](std::size_t idx) { return value[idx]; }
    };
#else
    typedef std::tr1::array<double, 16> Transform;
#endif
    Transform translate(double x, double y);
    Transform rotate(double angle, double aroundX = 0, double aroundY = 0);
    Transform scale(double factor);
    Transform scale(double factorX, double factorY, double fromX = 0, double fromY = 0);
    Transform concat(const Transform& lhs, const Transform& rhs);
    
#ifndef SWIG
    // A not so useful optimization - this was supposed to bypass the Z queue for immediate rendering.
    // In retrospect, the only useful optimization would be to work down the Z queue on a second thread.
    GOSU_DEPRECATED const double zImmediate = -std::numeric_limits<double>::infinity();
#endif
}

#endif
