#include <Gosu/Color.hpp>
#include <Gosu/Math.hpp>
#include <algorithm>

Gosu::Color Gosu::interpolate(Color a, Color b)
{
    Color result;
    result.setAlpha(round((a.alpha() + b.alpha()) / 2.0));
    result.setRed  (round((a.red()   + b.red())   / 2.0));
    result.setGreen(round((a.green() + b.green()) / 2.0));
    result.setBlue (round((a.blue()  + b.blue())  / 2.0));
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

Gosu::HSV Gosu::colorToHSV(const Color& c)
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

Gosu::Color Gosu::hsvToColor(const HSV& hsv)
{
	if (hsv.s == 0)
		// Grey.
        return Color(hsv.v * 255, hsv.v * 255, hsv.v * 255);
    
	int sector = static_cast<int>(hsv.h / 60);
    double factorial = hsv.h / 60 - sector;
    
	double p = hsv.v * (1 - hsv.s);
	double q = hsv.v * (1 - hsv.s * factorial);
	double t = hsv.v * (1 - hsv.s * (1 - factorial));
	
    switch (sector) 
    {
    case 0:
        return Color(hsv.v * 255, t * 255, p * 255);
    case 1:
        return Color(q * 255, hsv.v * 255, p * 255);
    case 2:
        return Color(p * 255, hsv.v * 255, t * 255);
    case 3:
        return Color(p * 255, q * 255, hsv.v * 255);
    case 4:
        return Color(t * 255, p * 255, hsv.v * 255);
    default: // sector 5
        return Color(hsv.v * 255, p * 255, q * 255);
	}
}
