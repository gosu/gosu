#include <Gosu/Math.hpp>
#include <cmath>
#include <cstdlib>

double Gosu::random(double min, double max)
{
    double rnd = std::rand();
    return rnd / RAND_MAX * (max - min) + min;
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
        return std::atan2(distY, distX) / pi * 180 + 90;
}

double Gosu::angleDiff(double angle1, double angle2)
{
    return std::fmod(std::fabs(angle1 - angle2), 360);
}

double Gosu::normalizeAngle(double angle)
{
    double result = std::fmod(angle, 360);
    if (result < 0)
        result += 360;
    return result;
}

double Gosu::distance(double x1, double y1, double x2, double y2)
{
    return std::sqrt(distanceSqr(x1, y1, x2, y2));
}
