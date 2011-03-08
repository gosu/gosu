#include <Gosu/Color.hpp>
#include <Gosu/Math.hpp>
#include <algorithm>

namespace
{
    struct HSV { double h, s, v; };
    
    HSV colorToHSV(const Gosu::Color& c)
    {
        double r = c.red() / 255.0;
        double g = c.green() / 255.0;
        double b = c.blue() / 255.0;
        
        double min = std::min(std::min(r, g), b);
        double max = std::max(std::max(r, g), b);

        if (max == 0)
        {
            HSV hsv = { 0, 0, 0 };
            return hsv;
        }
        
        // Value.
        HSV hsv = { -1, -1, max };
        
        // Saturation.
        double delta = max - min;
        hsv.s = delta / max;
        
        // Hue.
        if (r == max)
            hsv.h = (g - b) / delta + (g < b ? 6 : 0);
        else if (g == max)
            hsv.h = (b - r) / delta + 2;
        else
            hsv.h = (r - g) / delta + 4;
        hsv.h *= 60;
        
        return hsv;
    }
}

Gosu::Color Gosu::Color::fromHSV(double h, double s, double v)
{
    return fromAHSV(255, h, s, v);
}

Gosu::Color Gosu::Color::fromAHSV(Channel alpha, double h, double s, double v)
{
	if (s == 0)
		// Grey.
        return Color(alpha, v * 255, v * 255, v * 255);
    
    // Normalize hue
    h = normalizeAngle(h);
    
	int sector = h / 60;
    double factorial = h / 60 - sector;
    
	double p = v * (1 - s);
	double q = v * (1 - s * factorial);
	double t = v * (1 - s * (1 - factorial));
	
    switch (sector) 
    {
    case 0:
        return Color(alpha, v * 255, t * 255, p * 255);
    case 1:
        return Color(alpha, q * 255, v * 255, p * 255);
    case 2:
        return Color(alpha, p * 255, v * 255, t * 255);
    case 3:
        return Color(alpha, p * 255, q * 255, v * 255);
    case 4:
        return Color(alpha, t * 255, p * 255, v * 255);
    default: // sector 5
        return Color(alpha, v * 255, p * 255, q * 255);
    }
}

double Gosu::Color::hue() const
{
    return colorToHSV(*this).h;
}

void Gosu::Color::setHue(double h)
{
    *this = fromAHSV(alpha(), h, saturation(), value());
}

double Gosu::Color::saturation() const
{
    return colorToHSV(*this).s;
}

void Gosu::Color::setSaturation(double s)
{
    *this = fromAHSV(alpha(), hue(), s, value());
}

double Gosu::Color::value() const
{
    return colorToHSV(*this).v;
}

void Gosu::Color::setValue(double v)
{
    *this = fromAHSV(alpha(), hue(), saturation(), v);
}

Gosu::Color Gosu::interpolate(Color a, Color b, double weight)
{
    return Color(clamp<int>(round(Gosu::interpolate(a.alpha(), b.alpha(), weight)), 0, 255),
                 clamp<int>(round(Gosu::interpolate(a.red(),   b.red(),   weight)), 0, 255),
                 clamp<int>(round(Gosu::interpolate(a.green(), b.green(), weight)), 0, 255),
                 clamp<int>(round(Gosu::interpolate(a.blue(),  b.blue(),  weight)), 0, 255));
}

Gosu::Color Gosu::multiply(Color a, Color b)
{
    return Color(round(a.alpha() * b.alpha() / 255.0),
                 round(a.red()   * b.red()   / 255.0),
                 round(a.green() * b.green() / 255.0),
                 round(a.blue()  * b.blue()  / 255.0));
}

const Gosu::Color Gosu::Color::NONE    = 0x00000000;
const Gosu::Color Gosu::Color::BLACK   = 0xff000000;
const Gosu::Color Gosu::Color::GRAY    = 0xff808080;
const Gosu::Color Gosu::Color::WHITE   = 0xffffffff;
const Gosu::Color Gosu::Color::AQUA    = 0xff00ffff;
const Gosu::Color Gosu::Color::RED     = 0xffff0000;
const Gosu::Color Gosu::Color::GREEN   = 0xff00ff00;
const Gosu::Color Gosu::Color::BLUE    = 0xff0000ff;
const Gosu::Color Gosu::Color::YELLOW  = 0xffffff00;
const Gosu::Color Gosu::Color::FUCHSIA = 0xffff00ff;
const Gosu::Color Gosu::Color::CYAN    = 0xff00ffff;
