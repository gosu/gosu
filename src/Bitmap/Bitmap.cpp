#include <Gosu/Bitmap.hpp>
#include <cassert>
#include <algorithm>
#include <vector>

void Gosu::Bitmap::swap(Bitmap& other)
{
    std::swap(pixels, other.pixels);
    std::swap(w, other.w);
    std::swap(h, other.h);
}

void Gosu::Bitmap::resize(unsigned width, unsigned height, Color c)
{
    if (width == w && height == h)
        return;

    Bitmap temp(width, height, c);
    temp.insert(*this, 0, 0);
    swap(temp);
}

void Gosu::Bitmap::fill(Color c)
{
    std::fill(pixels.begin(), pixels.end(), c);
}

void Gosu::Bitmap::replace(Color what, Color with)
{
    std::replace(pixels.begin(), pixels.end(), what, with);
}

void Gosu::Bitmap::insert(const Bitmap& source, int x, int y)
{
    insert(source, x, y, 0, 0, source.width(), source.height());
}

void Gosu::Bitmap::insert(const Bitmap& source, int x, int y, unsigned srcX,
    unsigned srcY, unsigned srcWidth, unsigned srcHeight)
{
    // TODO: This should use memcpy if possible (x == 0 && srcWidth == this->width())

    if (x < 0)
    {
        unsigned clipLeft = -x;

        if (clipLeft >= srcWidth)
            return;

        srcX += clipLeft;
        srcWidth -= clipLeft;
        x = 0;
    }

    if (y < 0)
    {
        unsigned clipTop = -y;

        if (clipTop >= srcHeight)
            return;

        srcY += clipTop;
        srcHeight -= clipTop;
        y = 0;
    }

    if (x + srcWidth > w)
    {
        if (static_cast<unsigned>(x) >= w)
            return;

        srcWidth = w - x;
    }

    if (y + srcHeight > h)
    {
        if (static_cast<unsigned>(y) >= h)
            return;

        srcHeight = h - y;
    }

    for (unsigned relY = 0; relY < srcHeight; ++relY)
        for (unsigned relX = 0; relX < srcWidth; ++relX)
            setPixel(x + relX, y + relY,
                source.getPixel(srcX + relX, srcY + relY));
}

void Gosu::applyColorKey(Bitmap& bitmap, Color key)
{
    std::vector<Color> surroundingColors;
    surroundingColors.reserve(4);

    for (unsigned y = 0; y < bitmap.height(); ++y)
        for (unsigned x = 0; x < bitmap.width(); ++x)
            if (bitmap.getPixel(x, y) == key)
            {
                surroundingColors.clear();
                if (x > 0 && bitmap.getPixel(x - 1, y) != key)
                    surroundingColors.push_back(bitmap.getPixel(x - 1, y));
                if (x < bitmap.width() - 1 && bitmap.getPixel(x + 1, y) != key)
                    surroundingColors.push_back(bitmap.getPixel(x + 1, y));
                if (y > 0 && bitmap.getPixel(x, y - 1) != key)
                    surroundingColors.push_back(bitmap.getPixel(x, y - 1));
                if (y < bitmap.height() - 1 && bitmap.getPixel(x, y + 1) != key)
                    surroundingColors.push_back(bitmap.getPixel(x, y + 1));

                if (surroundingColors.empty())
                {
                    bitmap.setPixel(x, y, Color::NONE);
                    continue;
                }

                unsigned red = 0, green = 0, blue = 0;
                for (unsigned i = 0; i < surroundingColors.size(); ++i)
                {
                    red += surroundingColors[i].red();
                    green += surroundingColors[i].green();
                    blue += surroundingColors[i].blue();
                }
                bitmap.setPixel(x, y, Color(0, red / surroundingColors.size(),
                    green / surroundingColors.size(), blue / surroundingColors.size()));
            }
}

void Gosu::unapplyColorKey(Bitmap& bitmap, Color color)
{
    Color* p = bitmap.data();
    for (int i = bitmap.width() * bitmap.height(); i > 0; --i, ++p)
        if (p->alpha() == 0)
            *p = color;
        else
            p->setAlpha(255);
}

void Gosu::applyBorderFlags(Bitmap& dest, const Bitmap& source,
    unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
    unsigned imageFlags)
{
    // Backwards compatibility: This used to be 'bool tileable'.
    if (imageFlags == 1)
        imageFlags = ifTileable;

    dest.resize(srcWidth + 2, srcHeight + 2);

    // The borders are made "harder" by duplicating the original bitmap's
    // borders.

    // Top.
    if (imageFlags & ifTileableTop)
        dest.insert(source, 1, 0, srcX, srcY, srcWidth, 1);
    // Bottom.
    if (imageFlags & ifTileableBottom)
        dest.insert(source, 1, dest.height() - 1,
            srcX, srcY + srcHeight - 1, srcWidth, 1);
    // Left.
    if (imageFlags & ifTileableLeft)
        dest.insert(source, 0, 1, srcX, srcY, 1, srcHeight);
    // Right.
    if (imageFlags & ifTileableRight)
        dest.insert(source, dest.width() - 1, 1,
            srcX + srcWidth - 1, srcY, 1, srcHeight);

    // Top left.
    if ((imageFlags & ifTileableTop) && (imageFlags & ifTileableLeft))
        dest.setPixel(0, 0,
            source.getPixel(srcX, srcY));
    // Top right.
    if ((imageFlags & ifTileableTop) && (imageFlags & ifTileableRight))
        dest.setPixel(dest.width() - 1, 0,
            source.getPixel(srcX + srcWidth - 1, srcY));
    // Bottom left.
    if ((imageFlags & ifTileableBottom) && (imageFlags & ifTileableLeft))
        dest.setPixel(0, dest.height() - 1,
            source.getPixel(srcX, srcY + srcHeight - 1));
    // Bottom right.
    if ((imageFlags & ifTileableBottom) && (imageFlags & ifTileableRight))
        dest.setPixel(dest.width() - 1, dest.height() - 1,
            source.getPixel(srcX + srcWidth - 1, srcY + srcHeight - 1));

    // Now put the final image into the prepared borders.
    dest.insert(source, 1, 1, srcX, srcY, srcWidth, srcHeight);
}
