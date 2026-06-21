#include <Gosu/Color.hpp>
#include <Gosu/Math.hpp>
#include <algorithm> // for std::min, std::max
#include <cmath>
#include <iomanip> // for std::setw

Gosu::Color Gosu::Color::from_hsv(double h, double s, double v)
{
    // Normalize hue so that is always in the [0, 360) range and wraps around.
    h = normalize_angle(h);
    // Clamp s and v for consistency with the Ruby/Gosu ARGB getters/setters.
    s = std::clamp(s, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);

    const auto to_channel = [](double d) { return static_cast<Channel>(std::round(255.0 * d)); };

    int sector = static_cast<int>(h / 60);
    double remainder = h / 60 - sector;

    Channel p = to_channel(v * (1 - s));
    Channel q = to_channel(v * (1 - s * remainder));
    Channel t = to_channel(v * (1 - s * (1 - remainder)));

    switch (sector) {
    case 0:
        return Color { to_channel(v), t, p };
    case 1:
        return Color { q, to_channel(v), p };
    case 2:
        return Color { p, to_channel(v), t };
    case 3:
        return Color { p, q, to_channel(v) };
    case 4:
        return Color { t, p, to_channel(v) };
    default: // sector 5
        return Color { to_channel(v), p, q };
    }
}

double Gosu::Color::hue() const
{
    double max = std::max({ red, green, blue });
    double min = std::min({ red, green, blue });

    if (min == max) {
        return 0;
    }

    double factor = 60 / (max - min);

    if (green == max) {
        return (blue - red) * factor + 120; // 60...180
    } else if (blue == max) {
        return (red - green) * factor + 240; // 180...300
    } else if (blue > green) {
        return (green - blue) * factor + 360; // 300...360
    } else {
        return (green - blue) * factor + 0; // 0...60
    }
}

void Gosu::Color::set_hue(double h)
{
    *this = from_hsv(h, saturation(), value()).with_alpha(alpha);
}

double Gosu::Color::saturation() const
{
    double max = std::max({ red, green, blue });

    if (max == 0) {
        return 0;
    }

    double min = std::min({ red, green, blue });

    return 1 - (min / max);
}

void Gosu::Color::set_saturation(double s)
{
    *this = from_hsv(hue(), s, value()).with_alpha(alpha);
}

double Gosu::Color::value() const
{
    return std::max({ red, green, blue }) / 255.0;
}

void Gosu::Color::set_value(double v)
{
    *this = from_hsv(hue(), saturation(), v).with_alpha(alpha);
}

const Gosu::Color Gosu::Color::NONE { 0x00'000000 };
const Gosu::Color Gosu::Color::BLACK { 0, 0, 0 };
const Gosu::Color Gosu::Color::GRAY { 128, 128, 128 };
const Gosu::Color Gosu::Color::WHITE { 255, 255, 255 };

const Gosu::Color Gosu::Color::AQUA { 0, 255, 255 };
const Gosu::Color Gosu::Color::RED { 255, 0, 0 };
const Gosu::Color Gosu::Color::GREEN { 0, 255, 0 };
const Gosu::Color Gosu::Color::BLUE { 0, 0, 255 };
const Gosu::Color Gosu::Color::YELLOW { 255, 255, 0 };
const Gosu::Color Gosu::Color::FUCHSIA { 255, 0, 255 };
const Gosu::Color Gosu::Color::CYAN { 0, 255, 255 };

Gosu::Color Gosu::lerp(Color a, Color b, double t)
{
    const auto lerp_channel = [](Color::Channel a, Color::Channel b, double t) {
        return static_cast<Color::Channel>(std::clamp(std::round(std::lerp(a, b, t)), 0.0, 255.0));
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
    const auto multiply_channel = [](Color::Channel a, Color::Channel b) {
        return static_cast<Color::Channel>(std::round(a * b / 255.0));
    };

    Color result;
    result.red = multiply_channel(a.red, b.red);
    result.green = multiply_channel(a.green, b.green);
    result.blue = multiply_channel(a.blue, b.blue);
    result.alpha = multiply_channel(a.alpha, b.alpha);
    return result;
}

std::ostream& Gosu::operator<<(std::ostream& stream, Gosu::Color color)
{
    const auto previous_flags = stream.flags();
    const char previous_fill = stream.fill();
    stream << std::setfill('0');
    stream << "0x" << std::hex << std::setw(2) << static_cast<int>(color.alpha);
    stream << '\'' << std::setw(6) << static_cast<int>(color.argb() & 0x00'ffffff);
    stream.flags(previous_flags);
    stream.fill(previous_fill);
    return stream;
}
