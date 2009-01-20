#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>

#ifndef GOSU_IS_IPHONE
Gosu::Bitmap Gosu::quickLoadBitmap(const std::wstring& filename)
{
	Buffer buf;
	loadFile(buf, filename);
	Bitmap bmp;

    char formatTester[2];
    buf.frontReader().read(formatTester, sizeof formatTester);
    if (formatTester[0] == 'B' && formatTester[1] == 'M')
    {
        loadFromBMP(bmp, buf.frontReader());
        applyColorKey(bmp, Colors::fuchsia);
    }
    else
        loadFromPNG(bmp, buf.frontReader());
	return bmp;
}
#endif

void Gosu::applyBorderFlags(Bitmap& dest, const Bitmap& source,
    unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
    unsigned borderFlags)
{
    dest.resize(srcWidth + 2, srcHeight + 2);
    dest.fill(Colors::none);

    // The borders are made "harder" by duplicating the original bitmap's
    // borders.

    // Top.
    if (borderFlags & bfHardTop)
        dest.insert(source, 1, 0, srcX, srcY, srcWidth, 1);
    // Bottom.
    if (borderFlags & bfHardBottom)
        dest.insert(source, 1, dest.height() - 1,
            srcX, srcY + srcHeight - 1, srcWidth, 1);
    // Left.
    if (borderFlags & bfHardLeft)
        dest.insert(source, 0, 1, srcX, srcY, 1, srcHeight);
    // Right.
    if (borderFlags & bfHardRight)
        dest.insert(source, dest.width() - 1, 1, 
            srcX + srcWidth - 1, srcY, 1, srcHeight);

    // Top left.
    if ((borderFlags & bfHardTop) && (borderFlags & bfHardLeft))
        dest.setPixel(0, 0,
            source.getPixel(srcX, srcY));
    // Top right.
    if ((borderFlags & bfHardTop) && (borderFlags & bfHardRight))
        dest.setPixel(dest.width() - 1, 0,
            source.getPixel(srcX + srcWidth - 1, srcY));
    // Bottom left.
    if ((borderFlags & bfHardBottom) && (borderFlags & bfHardLeft))
        dest.setPixel(0, dest.height() - 1,
            source.getPixel(srcX, srcY + srcHeight - 1));
    // Bottom right.
    if ((borderFlags & bfHardBottom) && (borderFlags & bfHardRight))
        dest.setPixel(dest.width() - 1, dest.height() - 1,
            source.getPixel(srcX + srcWidth - 1, srcY + srcHeight - 1));

    // Now put the final image into the prepared borders.
    dest.insert(source, 1, 1, srcX, srcY, srcWidth, srcHeight);
}
