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

const Gosu::ImageData& Gosu::Image::getData() const
{
    return *data;
}
