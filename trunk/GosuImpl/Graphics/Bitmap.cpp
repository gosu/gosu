#include <Gosu/Bitmap.hpp>
#include <cassert>
#include <algorithm>
#include <vector>

Gosu::Bitmap::Bitmap()
: w(0), h(0)
{
}

/*unsigned Gosu::Bitmap::width() const
{
    return w;
}

unsigned Gosu::Bitmap::height() const
{
    return h;
}*/

void Gosu::Bitmap::swap(Bitmap& other)
{
    std::swap(pixels, other.pixels);
    std::swap(w, other.w);
    std::swap(h, other.h);
}

void Gosu::Bitmap::resize(unsigned width, unsigned height, Color c)
{
    Bitmap temp;
    temp.pixels.resize(width * height, c);
    temp.w = width;
    temp.h = height;
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

/*Gosu::Color Gosu::Bitmap::getPixel(unsigned x, unsigned y) const
{
    assert(x <= w);
    assert(y <= h);
    assert(w*h > 0);
    return pixels[y * w + x];
}

void Gosu::Bitmap::setPixel(unsigned x, unsigned y, Color c)
{
    assert(x <= w);
    assert(y <= h);
    assert(w*h > 0);
    pixels[y * w + x] = c;
}*/

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
