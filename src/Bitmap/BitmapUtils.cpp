#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>

Gosu::Reader Gosu::loadFromPNG(Bitmap& bitmap, Reader reader)
{
    loadImageFile(bitmap, reader);
    reader.setPosition(reader.resource().size());
    return reader;
}

Gosu::Reader Gosu::loadFromBMP(Bitmap& bitmap, Reader reader)
{
    return loadFromPNG(bitmap, reader);
}

Gosu::Writer Gosu::saveToPNG(const Bitmap& bitmap, Writer writer)
{
    saveImageFile(bitmap, writer);
    return writer.resource().backWriter();
}

// The iPhone still uses BitmapBMP.cpp because UIImage apparently lacks BMP export.
#ifndef GOSU_IS_IPHONE
Gosu::Writer Gosu::saveToBMP(const Bitmap& bitmap, Writer writer)
{
    saveImageFile(bitmap, writer, L"bmp");
    return writer.resource().backWriter();
}
#endif

void Gosu::applyBorderFlags(Bitmap& dest, const Bitmap& source,
    unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
    unsigned borderFlags)
{
    dest.resize(srcWidth + 2, srcHeight + 2);

    // The borders are made "harder" by duplicating the original bitmap's
    // borders.

    // Top.
    if (borderFlags & bfTileableTop)
        dest.insert(source, 1, 0, srcX, srcY, srcWidth, 1);
    // Bottom.
    if (borderFlags & bfTileableBottom)
        dest.insert(source, 1, dest.height() - 1,
            srcX, srcY + srcHeight - 1, srcWidth, 1);
    // Left.
    if (borderFlags & bfTileableLeft)
        dest.insert(source, 0, 1, srcX, srcY, 1, srcHeight);
    // Right.
    if (borderFlags & bfTileableRight)
        dest.insert(source, dest.width() - 1, 1, 
            srcX + srcWidth - 1, srcY, 1, srcHeight);

    // Top left.
    if ((borderFlags & bfTileableTop) && (borderFlags & bfTileableLeft))
        dest.setPixel(0, 0,
            source.getPixel(srcX, srcY));
    // Top right.
    if ((borderFlags & bfTileableTop) && (borderFlags & bfTileableRight))
        dest.setPixel(dest.width() - 1, 0,
            source.getPixel(srcX + srcWidth - 1, srcY));
    // Bottom left.
    if ((borderFlags & bfTileableBottom) && (borderFlags & bfTileableLeft))
        dest.setPixel(0, dest.height() - 1,
            source.getPixel(srcX, srcY + srcHeight - 1));
    // Bottom right.
    if ((borderFlags & bfTileableBottom) && (borderFlags & bfTileableRight))
        dest.setPixel(dest.width() - 1, dest.height() - 1,
            source.getPixel(srcX + srcWidth - 1, srcY + srcHeight - 1));

    // Now put the final image into the prepared borders.
    dest.insert(source, 1, 1, srcX, srcY, srcWidth, srcHeight);
}
