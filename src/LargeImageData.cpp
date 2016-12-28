#include "LargeImageData.hpp"
#include "GraphicsImpl.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Math.hpp>
#include <cmath>
using namespace std;

Gosu::LargeImageData::LargeImageData(const Bitmap& source,
    unsigned partWidth, unsigned partHeight, unsigned imageFlags)
{
    fullWidth = source.width();
    fullHeight = source.height();
    partsX = static_cast<unsigned>(trunc(ceil(1.0 * source.width() / partWidth)));
    partsY = static_cast<unsigned>(trunc(ceil(1.0 * source.height() / partHeight)));
    this->partWidth = partWidth;
    this->partHeight = partHeight;

    parts.resize(partsX * partsY);

    for (unsigned y = 0; y < partsY; ++y)
        for (unsigned x = 0; x < partsX; ++x)
        {
            // The right-most parts don't necessarily have the full width.
            unsigned srcWidth = partWidth;
            if (x == partsX - 1 && source.width() % partWidth != 0)
                srcWidth = source.width() % partWidth;

            // Same for the parts on the bottom.
            unsigned srcHeight = partHeight;
            if (y == partsY - 1 && source.height() % partHeight != 0)
                srcHeight = source.height() % partHeight;

            unsigned localFlags = ifTileable | imageFlags;
            if (x == 0)
                localFlags = (localFlags & ~ifTileableLeft) | (imageFlags & ifTileableLeft);
            if (x == partsX - 1)
                localFlags = (localFlags & ~ifTileableRight) | (imageFlags & ifTileableRight);
            if (y == 0)
                localFlags = (localFlags & ~ifTileableTop) | (imageFlags & ifTileableTop);
            if (y == partsY - 1)
                localFlags = (localFlags & ~ifTileableBottom) | (imageFlags & ifTileableBottom);
            
            parts[y * partsX + x].reset(Graphics::createImage(source,
                x * partWidth, y * partHeight, srcWidth, srcHeight,
                localFlags).release());
        }
}

int Gosu::LargeImageData::width() const
{
    return fullWidth;
}

int Gosu::LargeImageData::height() const
{
    return fullHeight;
}

namespace
{
    // Local interpolation helper functions.

    double ipl(double a, double b, double ratio)
    {
        return a + (b - a) * ratio;
    }

    Gosu::Color ipl(Gosu::Color a, Gosu::Color b, double ratio)
    {
        Gosu::Color result;
        result.setAlpha(Gosu::round(ipl(a.alpha(), b.alpha(), ratio)));
        result.setRed  (Gosu::round(ipl(a.red(),   b.red(),   ratio)));
        result.setGreen(Gosu::round(ipl(a.green(), b.green(), ratio)));
        result.setBlue (Gosu::round(ipl(a.blue(),  b.blue(),  ratio)));
        return result;
    }
}

void Gosu::LargeImageData::draw(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    double x4, double y4, Color c4, ZPos z, AlphaMode mode) const
{
    if (parts.empty())
        return;

    reorderCoordinatesIfNecessary(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    for (unsigned py = 0; py < partsY; ++py)
        for (unsigned px = 0; px < partsX; ++px)
        {
            ImageData& part = *parts[py * partsX + px];

            double relXL = static_cast<double>(px * partWidth) / width();
            double relXR = static_cast<double>(px * partWidth + part.width()) / width();
            double relYT = static_cast<double>(py * partHeight) / height();
            double relYB = static_cast<double>(py * partHeight + part.height()) / height();

            double absXTL = ipl(ipl(x1, x3, relYT), ipl(x2, x4, relYT), relXL);
            double absXTR = ipl(ipl(x1, x3, relYT), ipl(x2, x4, relYT), relXR);
            double absXBL = ipl(ipl(x1, x3, relYB), ipl(x2, x4, relYB), relXL);
            double absXBR = ipl(ipl(x1, x3, relYB), ipl(x2, x4, relYB), relXR);

            double absYTL = ipl(ipl(y1, y3, relYT), ipl(y2, y4, relYT), relXL);
            double absYTR = ipl(ipl(y1, y3, relYT), ipl(y2, y4, relYT), relXR);
            double absYBL = ipl(ipl(y1, y3, relYB), ipl(y2, y4, relYB), relXL);
            double absYBR = ipl(ipl(y1, y3, relYB), ipl(y2, y4, relYB), relXR);

            Color absCTL = ipl(ipl(c1, c3, relYT), ipl(c2, c4, relYT), relXL);
            Color absCTR = ipl(ipl(c1, c3, relYT), ipl(c2, c4, relYT), relXR);
            Color absCBL = ipl(ipl(c1, c3, relYB), ipl(c2, c4, relYB), relXL);
            Color absCBR = ipl(ipl(c1, c3, relYB), ipl(c2, c4, relYB), relXR);

            part.draw(absXTL, absYTL, absCTL, absXTR, absYTR, absCTR,
                absXBL, absYBL, absCBL, absXBR, absYBR, absCBR, z, mode);
        }
}

Gosu::Bitmap Gosu::LargeImageData::toBitmap() const
{
    Bitmap bitmap(width(), height());
    for (int x = 0; x < partsX; ++x)
        for (int y = 0; y < partsY; ++y)
            bitmap.insert(parts[y * partsX + x]->toBitmap(), x * partWidth, y * partHeight);
    return bitmap;
}

void Gosu::LargeImageData::insert(const Bitmap& bitmap, int atX, int atY)
{
    for (int x = 0; x < partsX; ++x)
        for (int y = 0; y < partsY; ++y)
            parts[y * partsX + x]->insert(bitmap, atX - x * partWidth, atY - y * partHeight);
}
