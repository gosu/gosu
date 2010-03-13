#include <Gosu/Image.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>

Gosu::Image::Image(Graphics& graphics, const std::wstring& filename, bool tileable)
{
	// Forward.
	Image(graphics, quickLoadBitmap(filename), tileable).data.swap(data);
}

Gosu::Image::Image(Graphics& graphics, const std::wstring& filename,
    unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
    bool tileable)
{
	// Forward.
	Image(graphics, quickLoadBitmap(filename), srcX, srcY, srcWidth, srcHeight, tileable).data.swap(data);
}

Gosu::Image::Image(Graphics& graphics, const Bitmap& source, bool tileable)
{
	// Forward.
	Image(graphics, source, 0, 0, source.width(), source.height(), tileable).data.swap(data);
}

Gosu::Image::Image(Graphics& graphics, const Bitmap& source,
    unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
    bool tileable)
: data(graphics.createImage(source, srcX, srcY, srcWidth, srcHeight,
    tileable ? Gosu::bfTileable : Gosu::bfSmooth))
{
}

Gosu::Image::Image(std::auto_ptr<ImageData> data)
: data(data.release())
{
}

Gosu::Image::~Image()
{
}

unsigned Gosu::Image::width() const
{
    return data->width();
}

unsigned Gosu::Image::height() const
{
    return data->height();
}

void Gosu::Image::draw(double x, double y, ZPos z,
    double factorX, double factorY,
    Color c,
    AlphaMode mode) const
{
    double x2 = x + width() * factorX;
    double y2 = y + height() * factorY;

    data->draw(x, y, c, x2, y, c, x, y2, c, x2, y2, c, z, mode);
}

void Gosu::Image::drawMod(double x, double y, ZPos z,
    double factorX, double factorY,
    Color c1, Color c2, Color c3, Color c4,
    AlphaMode mode) const
{
    double x2 = x + width() * factorX;
    double y2 = y + height() * factorY;

    data->draw(x, y, c1, x2, y, c2, x, y2, c3, x2, y2, c4, z, mode);
}

void Gosu::Image::drawRot(double x, double y, ZPos z,
    double angle, double centerX, double centerY,
    double factorX, double factorY,
    Color c,
    AlphaMode mode) const
{
    double sizeX = width()  * factorX;
    double sizeY = height() * factorY;
    double offsX = offsetX(angle, 1);
    double offsY = offsetY(angle, 1);

    // Offset to the centers of the original Image's edges when it is rotated
    // by <angle> degrees.
    double distToLeftX   = +offsY * sizeX * centerX;
    double distToLeftY   = -offsX * sizeX * centerX;
    double distToRightX  = -offsY * sizeX * (1 - centerX);
    double distToRightY  = +offsX * sizeX * (1 - centerX);
    double distToTopX    = +offsX * sizeY * centerY;
    double distToTopY    = +offsY * sizeY * centerY;
    double distToBottomX = -offsX * sizeY * (1 - centerY);
    double distToBottomY = -offsY * sizeY * (1 - centerY);

    data->draw(x + distToLeftX  + distToTopX,
               y + distToLeftY  + distToTopY, c,
               x + distToRightX + distToTopX,
               y + distToRightY + distToTopY, c,
               x + distToLeftX  + distToBottomX,
               y + distToLeftY  + distToBottomY, c,
               x + distToRightX + distToBottomX,
               y + distToRightY + distToBottomY,
               c, z, mode);
}

void Gosu::Image::drawRotFlip(double x, double y, ZPos z,
    RotFlip rf,
    double factorX, double factorY,
    Color c,
    AlphaMode mode) const
{
    drawRotFlipMod(x, y, z, rf, factorX, factorY, c, c, c, c, mode);
}

void Gosu::Image::drawRotFlipMod(double x, double y, ZPos z,
    RotFlip rf,
    double factorX, double factorY,
    Color c1, Color c2, Color c3, Color c4,
    AlphaMode mode) const
{
    double offsetX;
    double offsetY;

    if (rf.rotated())
    {
        offsetX = height() * factorY / 2;
        offsetY = width() * factorX / 2;
    }
    else
    {
        offsetX = width() * factorX / 2;
        offsetY = height() * factorY / 2;
    }

    struct Point
    {
        double x, y;
    };

    Point corners[4] = { { x - offsetX, y - offsetY },
                         { x + offsetX, y - offsetY },
                         { x - offsetX, y + offsetY },
                         { x + offsetX, y + offsetY } };

    Point* vertices[4] =
        { &corners[rf.mapCorner(0)], &corners[rf.mapCorner(1)],
          &corners[rf.mapCorner(2)], &corners[rf.mapCorner(3)] };

    data->draw(
        vertices[0]->x, vertices[0]->y, c1,
        vertices[1]->x, vertices[1]->y, c2,
        vertices[2]->x, vertices[2]->y, c3,
        vertices[3]->x, vertices[3]->y, c4,
        z, mode);
}

const Gosu::ImageData& Gosu::Image::getData() const
{
    return *data;
}
