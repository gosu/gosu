#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#import <UIKit/UIKit.h>

namespace Gosu
{
    void uiImageToBitmap(UIImage* image, Bitmap& bitmap)
    {
        CGImageRef imageRef = [image CGImage];
        
        bitmap.resize(CGImageGetWidth(imageRef), CGImageGetHeight(imageRef));
        
        // Use a temporary context to draw the CGImage to the buffer.
        CGContextRef context =
            CGBitmapContextCreate(bitmap.data(),
                                  bitmap.width(), bitmap.height(), 8, bitmap.width() * 4,
                                  CGImageGetColorSpace(imageRef),
                                  kCGImageAlphaPremultipliedLast);
        CGContextDrawImage(context, CGRectMake(0.0, 0.0, bitmap.width(), bitmap.height()), imageRef);
        CGContextRelease(context);
    }
}

Gosu::Reader Gosu::loadFromBMP(Bitmap& bitmap, Reader reader)
{
    reader = loadFromPNG(bitmap, reader);
    applyColorKey(bitmap, Color::FUCHSIA);
    return reader;
}

Gosu::Reader Gosu::loadFromPNG(Bitmap& bitmap, Reader reader)
{
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);

    std::size_t length = reader.resource().size() - reader.position();
    ObjRef<NSMutableData> buffer([[NSMutableData alloc] initWithLength: length]);
    reader.read([buffer.get() mutableBytes], length);
    
    ObjRef<UIImage> image([[UIImage alloc] initWithData: buffer.get()]);
    uiImageToBitmap(image.obj(), bitmap);    
    return reader;
}

Gosu::Writer Gosu::saveToPNG(const Bitmap& bmp, Writer writer)
{
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);

    CGDataProviderRef dataProvider =
        CGDataProviderCreateWithData(0, bmp.data(), bmp.width() * bmp.height() * 4, 0);

    static CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    
    CGImageRef imageRef =
        CGImageCreate(bmp.width(), bmp.height(), 8, 32, bmp.width() * 4, colorspace,
                      kCGImageAlphaLast, dataProvider, 0, false, kCGRenderingIntentDefault);

    ObjRef<UIImage> image([[UIImage alloc] initWithCGImage: imageRef]);
    
    NSData* pngRef = UIImagePNGRepresentation(image.get());
    writer.write([pngRef bytes], [pngRef length]);
    image.reset();
    
    CGImageRelease(imageRef);

    CGDataProviderRelease(dataProvider);
    
    return writer;
}

Gosu::Bitmap Gosu::loadImageFile(Gosu::Reader reader)
{
    Bitmap bitmap;
    loadFromBMP(bitmap, reader);
    return bitmap;
}

Gosu::Bitmap Gosu::loadImageFile(const std::wstring& filename)
{
    ObjRef<NSString> filenameRef([[NSString alloc] initWithUTF8String: wstringToUTF8(filename).c_str()]);
    ObjRef<UIImage> image([[UIImage alloc] initWithContentsOfFile: filenameRef.obj()]);
    Bitmap bitmap;
    uiImageToBitmap(image.obj(), bitmap);
    if (boost::iends_with(filename, L".bmp"))
        applyColorKey(bitmap, Color::FUCHSIA);
    return bitmap;
}
