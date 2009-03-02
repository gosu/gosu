#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <boost/algorithm/string.hpp>
#include <stdexcept>
#import <UIKit/UIKit.h>

Gosu::Bitmap Gosu::quickLoadBitmap(const std::wstring& filename)
{
    if (boost::iends_with(filename, L".bmp"))
    {
        Bitmap result;
        Buffer file;
        loadFile(file, filename);
        loadFromBMP(result, file.frontReader());
        applyColorKey(result, 0xffff00ff);
        return result;
    }

    // First create a UIImage object from the data in a image file, and then extract the Core Graphics image
    std::string utf8Filename = wstringToUTF8(filename);
    CGImageRef image = [UIImage imageNamed: [NSString stringWithUTF8String:utf8Filename.c_str()]].CGImage;
    if (!image)
        throw std::runtime_error("Could not load image " + utf8Filename);
    
    // Get the width and height of the image
    Bitmap bmp;
    bmp.resize(CGImageGetWidth(image), CGImageGetHeight(image));
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGContextRef context =
        CGBitmapContextCreate(bmp.data(),
                              bmp.width(), bmp.height(), 8, bmp.width() * 4,
                              CGImageGetColorSpace(image),
                              kCGImageAlphaPremultipliedLast);
    CGContextDrawImage(context, CGRectMake(0.0, 0.0, bmp.width(), bmp.height()), image);
    CGContextRelease(context);
    
    for (unsigned y = 0; y < bmp.height(); ++y)
        for (unsigned x = 0; x < bmp.width(); ++x)
            bmp.setPixel(x, y, bmp.getPixel(x, y).abgr());
    
    // Done!
    return bmp;
}
