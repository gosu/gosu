//! \file GraphicsBase.hpp
//! Contains general typedefs and enums related to graphics.

#ifndef GOSU_GRAPHICSBASE_HPP
#define GOSU_GRAPHICSBASE_HPP

#include <Gosu/Platform.hpp>
#include <limits>
#if defined(GOSU_CPP11_ENABLED)
#include <type_traits>
#endif

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
    
#if defined(GOSU_CPP11_ENABLED)
    enum class FontFlags
    {
        NONE = 0,
        BOLD = 1,
        ITALIC = 2,
        UNDERLINE = 4
    };
    static const typename std::underlying_type<FontFlags>::type FontFlagCombinations = 8;
    GOSU_DEPRECATED static const typename std::underlying_type<FontFlags>::type ffCombinations = FontFlagCombinations;
    GOSU_DEPRECATED static const FontFlags ffNone = FontFlags::NONE;
    GOSU_DEPRECATED static const FontFlags ffBold = FontFlags::BOLD;
    GOSU_DEPRECATED static const FontFlags ffItalic = FontFlags::ITALIC;
    GOSU_DEPRECATED static const FontFlags ffUnderline = FontFlags::UNDERLINE;
    inline FontFlags operator|(const FontFlags& lhs, const FontFlags& rhs)
    {
        typedef typename std::underlying_type<FontFlags>::type int_type;
        return static_cast<FontFlags>(static_cast<int_type>(lhs)|static_cast<int_type>(rhs));
    }
    inline FontFlags operator&(const FontFlags& lhs, const FontFlags& rhs)
    {
        typedef typename std::underlying_type<FontFlags>::type int_type;
        return static_cast<FontFlags>(static_cast<int_type>(lhs)&static_cast<int_type>(rhs));
    }
    inline FontFlags& operator|=(FontFlags& lhs, const FontFlags& rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }
    inline FontFlags& operator&=(FontFlags& lhs, const FontFlags& rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }
    inline FontFlags operator~(const FontFlags& rhs)
    {
        typedef typename std::underlying_type<FontFlags>::type int_type;
        return static_cast<FontFlags>(~static_cast<int_type>(rhs));
    }
#else
    enum FontFlags
    {
        ffNone         = 0,
        ffBold         = 1,
        ffItalic       = 2,
        ffUnderline    = 4
    };
    static const size_t ffCombinations = 8;
#endif
    
#if defined(GOSU_CPP11_ENABLED)
    enum class TextAlign
    {
        Left,
        Right,
        Center,
        Justify
    };
    GOSU_DEPRECATED static const TextAlign taLeft = TextAlign::Left;
    GOSU_DEPRECATED static const TextAlign taRight = TextAlign::Right;
    GOSU_DEPRECATED static const TextAlign taCenter = TextAlign::Center;
    GOSU_DEPRECATED static const TextAlign taJustify = TextAlign::Justify;
#else
    enum TextAlign
    {
        taLeft,
        taRight,
        taCenter,
        taJustify
    };
#endif
    
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
    
    #ifndef SWIG
    // A not so useful optimization.
    GOSU_DEPRECATED const double zImmediate = -std::numeric_limits<double>::infinity();
    #endif
}

#endif
