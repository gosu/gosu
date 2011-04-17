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
