#include <Gosu/Color.hpp>
#include <Gosu/Math.hpp>
#include <algorithm>
#include <cmath>

namespace
{
    struct HSV
    {
        double h, s, v;
    };

    HSV color_to_hsv(const Gosu::Color& c)
    {
        double r = c.red / 255.0;
        double g = c.green / 255.0;
        double b = c.blue / 255.0;

        double min = std::min(std::min(r, g), b);
        double max = std::max(std::max(r, g), b);
        double delta = max - min;

        if (max == 0) {
            HSV hsv = {0, 0, 0};
            return hsv;
        }

        HSV hsv{};

        // Value.
        hsv.v = max;

        // Saturation.
        hsv.s = delta / max;

        // Hue.
        if (delta == 0) {
            hsv.h = 0;
        }
        else if (r == max) {
            hsv.h = (g - b) / delta + (g < b ? 6 : 0);
        }
        else if (g == max) {
            hsv.h = (b - r) / delta + 2;
        }
        else {
            hsv.h = (r - g) / delta + 4;
        }
        hsv.h *= 60;

        return hsv;
    }
}

Gosu::Color Gosu::Color::from_hsv(double h, double s, double v)
{
    // Normalize hue so that is always in the [0, 360) range and wraps around.
    h = normalize_angle(h);
    // Clamp s and v for consistency with the Ruby/Gosu ARGB getters/setters.
    s = std::clamp(s, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);

    int sector = static_cast<int>(h / 60);
    double factorial = h / 60 - sector;

    Channel p = static_cast<Channel>(255 * v * (1 - s));
    Channel q = static_cast<Channel>(255 * v * (1 - s * factorial));
    Channel t = static_cast<Channel>(255 * v * (1 - s * (1 - factorial)));

    switch (sector) {
    case 0:
        return Color{static_cast<Channel>(255 * v), t, p};
    case 1:
        return Color{q, static_cast<Channel>(255 * v), p};
    case 2:
        return Color{p, static_cast<Channel>(255 * v), t};
    case 3:
        return Color{p, q, static_cast<Channel>(255 * v)};
    case 4:
        return Color{t, p, static_cast<Channel>(255 * v)};
    default: // sector 5
        return Color{static_cast<Channel>(255 * v), p, q};
    }
}

double Gosu::Color::hue() const
{
    return color_to_hsv(*this).h;
}

void Gosu::Color::set_hue(double h)
{
    *this = from_hsv(h, saturation(), value()).with_alpha(alpha);
}

double Gosu::Color::saturation() const
{
    return color_to_hsv(*this).s;
}

void Gosu::Color::set_saturation(double s)
{
    *this = from_hsv(hue(), s, value()).with_alpha(alpha);
}

double Gosu::Color::value() const
{
    return color_to_hsv(*this).v;
}

void Gosu::Color::set_value(double v)
{
    *this = from_hsv(hue(), saturation(), v).with_alpha(alpha);
}

Gosu::Color Gosu::lerp(Color a, Color b, double t)
{
    const auto lerp_channel = [](Color::Channel a, Color::Channel b, double t) {
        return static_cast<Color::Channel>(std::clamp(std::round(lerp(a, b, t)), 0.0, 255.0));
    };

    Color result;
    result.red = lerp_channel(a.red, b.red, t);
    result.green = lerp_channel(a.green, b.green, t);
    result.blue = lerp_channel(a.blue, b.blue, t);
    result.alpha = lerp_channel(a.alpha, b.alpha, t);
    return result;
}

Gosu::Color Gosu::multiply(Color a, Color b)
{
    Color result;
    result.red = static_cast<Color::Channel>(std::round(a.red * b.red / 255.0));
    result.green = static_cast<Color::Channel>(std::round(a.green * b.green / 255.0));
    result.blue = static_cast<Color::Channel>(std::round(a.blue * b.blue / 255.0));
    result.alpha = static_cast<Color::Channel>(std::round(a.alpha * b.alpha / 255.0));
    return result;
}

const Gosu::Color Gosu::Color::NONE{0x00'000000};
const Gosu::Color Gosu::Color::BLACK{0, 0, 0};
const Gosu::Color Gosu::Color::GRAY{128, 128, 128};
const Gosu::Color Gosu::Color::WHITE{255, 255, 255};

const Gosu::Color Gosu::Color::AQUA{0, 255, 255};
const Gosu::Color Gosu::Color::RED{255, 0, 0};
const Gosu::Color Gosu::Color::GREEN{0, 255, 0};
const Gosu::Color Gosu::Color::BLUE{0, 0, 255};
const Gosu::Color Gosu::Color::YELLOW{255, 255, 0};
const Gosu::Color Gosu::Color::FUCHSIA{255, 0, 255};
const Gosu::Color Gosu::Color::CYAN{0, 255, 255};
