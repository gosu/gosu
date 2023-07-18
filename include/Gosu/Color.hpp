#pragma once

#include <compare>
#include <cstdint>
#include <ostream>

namespace Gosu
{
    /// Color represents an RGBA 32-bit color value, using 8 bit for each channel.
    /// Colors can be implicitly constructed from literals of the form 0xaarrggbb.
    struct Color
    {
        static const unsigned GL_FORMAT = 0x1908; // GL_RGBA

        using Channel = std::uint8_t;
        Channel red = 0, green = 0, blue = 0, alpha = 0;

        Color() = default;

        /// Implicit conversion constructor for literals of the form 0xaarrggbb.
        // NOLINTNEXTLINE: We want to allow implicit conversions.
        Color(std::uint32_t argb)
            : red(argb >> 16),
              green(argb >> 8),
              blue(argb >> 0),
              alpha(argb >> 24)
        {
        }

        Color(Channel red, Channel green, Channel blue)
            : red(red),
              green(green),
              blue(blue),
              alpha(255)
        {
        }

        Color with_alpha(Channel new_alpha) const
        {
            Color result = *this;
            result.alpha = new_alpha;
            return result;
        }

        /// Constructs a color from the given hue/saturation/value triple.
        /// s and v will be clamped into the range 0...1, and h will be normalized to 0...360.
        static Color from_hsv(double h, double s, double v);

        /// Returns the hue of the color, in the usual range of 0..360.
        double hue() const;

        /// Changes the current color so hue() will return h.
        /// Note: This is a no-op when called on a grey tone (saturation() == 0).
        void set_hue(double h);

        /// Returns the saturation of the color, in the range of 0..1.
        double saturation() const;

        /// Changes the current color so saturation() will return s.
        void set_saturation(double s);

        /// Returns the value (brightness) of the color, in the range of 0..1.
        double value() const;

        /// Changes the current color so value() will return v.
        void set_value(double v);

        /// Returns the color in 0xaarrggbb representation.
        std::uint32_t argb() const { return alpha << 24 | red << 16 | green << 8 | blue; }

        /// Returns the color in 0xbbggrr representation. Useful for Win32 programming, as this
        /// corresponds to the COLORREF type that can be created using the RGB() macro.
        std::uint32_t bgr() const { return blue << 16 | green << 8 | red; }

        /// Returns the color in 0xaabbggrr representation.
        std::uint32_t abgr() const { return alpha << 24 | blue << 16 | green << 8 | red; }

        /// Returns the internal representation of the color (RGBA in memory).
        std::uint32_t gl() const { return *reinterpret_cast<const std::uint32_t*>(this); }

        static const Color NONE;
        static const Color BLACK;
        static const Color GRAY;
        static const Color WHITE;

        /// Same as Color::CYAN.
        static const Color AQUA;
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color YELLOW;
        static const Color FUCHSIA;
        /// Same as Color::AQUA.
        static const Color CYAN;

        std::strong_ordering operator<=>(const Color&) const = default;
    };

    // Ensure that we can pass vectors of Gosu::Color straight to OpenGL and back.
    static_assert(sizeof(Color) == sizeof(std::uint32_t));

#ifndef SWIG
    /// Interpolates linearly between two colors, with a given weight towards
    /// the second color.
    Color lerp(Color a, Color b, double t = 0.5);

    /// Combines two colors as if their channels were mapped to the 0..1 range
    /// and then multiplied with each other.
    Color multiply(Color a, Color b);

    std::ostream& operator<<(std::ostream& stream, Color color);
#endif
}
