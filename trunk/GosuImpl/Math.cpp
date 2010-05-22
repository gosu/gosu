#include <Gosu/Math.hpp>
#include <cmath>
#include <cstdlib>

double Gosu::random(double min, double max)
{
    double rnd = std::rand();
    return rnd / (static_cast<double>(RAND_MAX) + 1) * (max - min) + min;
}

double Gosu::offsetX(double angle, double radius)
{
    return +std::sin(angle / 180 * pi) * radius;
}

double Gosu::offsetY(double angle, double radius)
{
    return -std::cos(angle / 180 * pi) * radius;
}

double Gosu::angle(double fromX, double fromY, double toX, double toY,
    double def)
{
    double distX = toX - fromX;
    double distY = toY - fromY;

    if (distX == 0 && distY == 0)
        return def;
    else
        return normalizeAngle(radiansToGosu(std::atan2(distY, distX)));
}

double Gosu::angleDiff(double from, double to)
{
    return Gosu::normalizeAngle(to - from + 180) - 180;
}

double Gosu::normalizeAngle(double angle)
{
    return wrap(angle, 0.0, 360.0);
}

int Gosu::wrap(int value, int min, int max)
{
    int result = (value - min) % (max - min);
    if (result < 0)
        return result + max;
    else
        return result + min;
}

float Gosu::wrap(float value, float min, float max)
{
    double result = std::fmod(value - min, max - min);
    if (result < 0)
        return result + max;
    else
        return result + min;
}

double Gosu::wrap(double value, double min, double max)
{
    double result = std::fmod(value - min, max - min);
    if (result < 0)
        return result + max;
    else
        return result + min;
}

double Gosu::distance(double x1, double y1, double x2, double y2)
{
    return std::sqrt(distanceSqr(x1, y1, x2, y2));
}
