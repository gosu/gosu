#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#import <UIKit/UIKit.h>

Gosu::Reader Gosu::loadFromBMP(Bitmap& bmp, Reader reader)
{
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);

    std::size_t length = reader.resource().size() - reader.position();
    ObjRef<NSMutableData> buffer([[NSMutableData alloc] initWithLength: length]);
    reader.read([buffer.get() mutableBytes], length);
    
    ObjRef<UIImage> image([[UIImage alloc] initWithData: buffer.get()]);
    if (!image.get())
        throw std::runtime_error("Could not load image from resource of length " +
                                 boost::lexical_cast<std::string>(length));
    
    CGImageRef imageRef = [image.obj() CGImage];
    
    bmp.resize(CGImageGetWidth(imageRef), CGImageGetHeight(imageRef));
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGContextRef context =
        CGBitmapContextCreate(bmp.data(),
                              bmp.width(), bmp.height(), 8, bmp.width() * 4,
                              CGImageGetColorSpace(imageRef),
                              kCGImageAlphaPremultipliedLast);
    CGContextDrawImage(context, CGRectMake(0.0, 0.0, bmp.width(), bmp.height()), imageRef);
    CGContextRelease(context);
    
    for (unsigned y = 0; y < bmp.height(); ++y)
        for (unsigned x = 0; x < bmp.width(); ++x)
            bmp.setPixel(x, y, bmp.getPixel(x, y).abgr());
    
    return reader;
}

Gosu::Reader Gosu::loadFromPNG(Bitmap& bmp, Reader reader)
{
    return loadFromBMP(bmp, reader);
}

Gosu::Writer Gosu::saveToPNG(const Bitmap& bmp, Writer writer)
{
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);

    CGDataProviderRef dataProvider =
        CGDataProviderCreateWithData(0, bmp.data(), bmp.width() * bmp.height() * 4, 0);

    static CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();

    CGImageRef imageRef =
        CGImageCreate(bmp.width(), bmp.height(), 8, 32, bmp.width() * 4, colorspace,
                      kCGImageAlphaFirst, dataProvider, 0, false, kCGRenderingIntentDefault);

    ObjRef<UIImage> image([[UIImage alloc] initWithCGImage: imageRef]);
    
    NSData* pngRef = UIImagePNGRepresentation(image.get());
    writer.write([pngRef bytes], [pngRef length]);
    image.reset();
    
    CGImageRelease(imageRef);

    CGDataProviderRelease(dataProvider);
    
    return writer;
}
