#include <Gosu/Color.hpp>
#include <Gosu/Math.hpp>
#include <algorithm>

namespace
{
    struct HSV { double h, s, v; };
    
    HSV colorToHSV(const Gosu::Color& c)
    {
        HSV hsv;
        
        double r = c.red() / 255.0;
        double g = c.green() / 255.0;
        double b = c.blue() / 255.0;
        
        double min = std::min(std::min(r, g), b);
        double max = std::max(std::max(r, g), b);

        if (max == 0)
        {
            hsv.h = hsv.s = hsv.v = 0;
            return hsv;
        }
        
        // Value.
        hsv.v = max;

        double delta = max - min;
        
        // Saturation.
        hsv.s = delta / max;
        
        // Hue.
        if (r == max)
            hsv.h = (g - b) / delta;
        else if (g == max)
            hsv.h = 2 + (b - r) / delta;
        else
            hsv.h = 4 + (r - g) / delta;
        hsv.h *= 60;
        if (hsv.h < 0)
            hsv.h += 360;
        
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
    
	int sector = static_cast<int>(h / 60);
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
    Color result;
    result.setAlpha(round((a.alpha() + b.alpha()) * weight));
    result.setRed  (round((a.red()   + b.red())   * weight));
    result.setGreen(round((a.green() + b.green()) * weight));
    result.setBlue (round((a.blue()  + b.blue())  * weight));
    return result;
}

Gosu::Color Gosu::multiply(Color a, Color b)
{
    Color result;
    result.setAlpha(round(a.alpha() * b.alpha() / 255.0));
    result.setRed  (round(a.red()   * b.red()   / 255.0));
    result.setGreen(round(a.green() * b.green() / 255.0));
    result.setBlue (round(a.blue()  * b.blue()  / 255.0));
    return result;
}
