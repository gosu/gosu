#if defined(GOSU_IS_IPHONE) || defined(__LP64__)

#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>

#ifdef GOSU_IS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

namespace Gosu
{
    void CGImageToBitmap(CGImageRef imageRef, Bitmap& bitmap)
    {
        bitmap.resize(CGImageGetWidth(imageRef), CGImageGetHeight(imageRef));
        
        static CFRef<CGColorSpaceRef> colorSpace(CGColorSpaceCreateDeviceRGB());
        
        // Use a temporary context to draw the CGImage to the buffer.
        CFRef<CGContextRef> context(CGBitmapContextCreate(bitmap.data(), bitmap.width(), bitmap.height(),
                                        8, bitmap.width() * 4, colorSpace.obj(), kCGImageAlphaPremultipliedLast));
        CGContextDrawImage(context.obj(), CGRectMake(0.0, 0.0, bitmap.width(), bitmap.height()), imageRef);
    }

    #ifdef GOSU_IS_IPHONE
    #define APPLE_IMAGE UIImage
    void appleImageToBitmap(UIImage* image, Bitmap& bitmap)
    {
        CGImageToBitmap([image CGImage], bitmap);
    }
    #else
    #define APPLE_IMAGE NSImage
    void appleImageToBitmap(NSImage* image, Bitmap& bitmap)
    {
        //#ifdef __LP64__
        CGImageToBitmap([image CGImageForProposedRect:NULL context:nil hints:nil], bitmap);
        //#else
        
        // The code below causes colors to be slightly off on my machine (64-bit, 10.6), but should
        // do the same thing and work on 10.4 & 10.5.
        // This may be due to http://stackoverflow.com/questions/2239785/nsimage-color-shift-on-snow-leopard
        // TODO: As soon as I have a 10.4/10.5 machine to verify, use above code on 10.6 and the code
        // below otherwise. If it works, BitmapBMP and BitmapPNG can be kicked out on OS X.
        
        // id anyRep = [image bestRepresentationForDevice:nil];
        // if (![anyRep isKindOfClass:[NSBitmapImageRep class]])
        //     throw std::logic_error("Cannot read vector graphics files");
        // NSBitmapImageRep* rep = (NSBitmapImageRep*)anyRep;
        // 
        // bitmap.resize([rep pixelsWide], [rep pixelsHigh]);
        // 
        // CFRef<CGColorSpaceRef> colorSpace(CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear));
        // 
        // // Use a temporary context to draw the NSImage to the buffer.
        // CFRef<CGContextRef> context(CGBitmapContextCreate(bitmap.data(), bitmap.width(), bitmap.height(),
        //                             8, bitmap.width() * 4,
        //                             colorSpace.obj(), kCGImageAlphaPremultipliedLast)); // kCGBitmapByteOrder32Host?
        // [NSGraphicsContext saveGraphicsState];
        // [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:context.obj() flipped:NO]];
        // [image drawInRect:NSMakeRect(0, 0, bitmap.width(), bitmap.height()) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
        // [NSGraphicsContext restoreGraphicsState];
        //#endif
    }
    #endif
}

Gosu::Reader Gosu::loadFromBMP(Bitmap& bitmap, Reader reader)
{
    return loadFromPNG(bitmap, reader);
}

Gosu::Reader Gosu::loadFromPNG(Bitmap& bitmap, Reader reader)
{
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);

    std::size_t length = reader.resource().size() - reader.position();
    ObjRef<NSMutableData> buffer([[NSMutableData alloc] initWithLength: length]);
    reader.read([buffer.get() mutableBytes], length);
    
    ObjRef<APPLE_IMAGE> image([[APPLE_IMAGE alloc] initWithData: buffer.get()]);
    appleImageToBitmap(image.obj(), bitmap);    
    return reader;
}

// OS X still completely uses the libpng based saveToPNG.
#ifdef GOSU_IS_IPHONE
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
#endif

Gosu::Bitmap Gosu::loadImageFile(const std::wstring& filename)
{
    ObjRef<NSString> filenameRef([[NSString alloc] initWithUTF8String: wstringToUTF8(filename).c_str()]);
    ObjRef<APPLE_IMAGE> image([[APPLE_IMAGE alloc] initWithContentsOfFile: filenameRef.obj()]);
    Bitmap bitmap;
    appleImageToBitmap(image.obj(), bitmap);
    if (boost::iends_with(filename, L".bmp"))
        applyColorKey(bitmap, Color::FUCHSIA);
    return bitmap;
}
#endif
