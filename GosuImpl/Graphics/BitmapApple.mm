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
#import <AppKit/NSStringDrawing.h>
#endif

namespace
{
    void CGImageToBitmap(CGImageRef imageRef, Gosu::Bitmap& bitmap)
    {
        static Gosu::CFRef<CGColorSpaceRef> colorSpace(CGColorSpaceCreateDeviceRGB());
        
        // Use a temporary context to draw the CGImage into the bitmap.
        bitmap.resize(CGImageGetWidth(imageRef), CGImageGetHeight(imageRef));
        Gosu::CFRef<CGContextRef> context(CGBitmapContextCreate(bitmap.data(), bitmap.width(), bitmap.height(),
                                        8, bitmap.width() * 4, colorSpace.obj(), kCGImageAlphaPremultipliedLast));
        CGContextDrawImage(context.obj(), CGRectMake(0.0, 0.0, bitmap.width(), bitmap.height()), imageRef);
    }
    
    #ifdef GOSU_IS_IPHONE
    #define APPLE_IMAGE UIImage
    void appleImageToBitmap(UIImage* image, Gosu::Bitmap& bitmap)
    {
        CGImageToBitmap([image CGImage], bitmap);
    }
    #else
    #define APPLE_IMAGE NSImage
    void appleImageToBitmap(NSImage* image, Gosu::Bitmap& bitmap)
    {
        // If we have CGImageForProposedRect (10.6+), use it. This is important because the code below
        // started to break in 10.6:
           http://stackoverflow.com/questions/2239785/nsimage-color-shift-on-snow-leopard

        if ([image respondsToSelector:@selector(CGImageForProposedRect:context:hints:)])
            return CGImageToBitmap((CGImageRef)[image CGImageForProposedRect:NULL context:nil hints:nil], bitmap);
        
        // Otherwise, take the slow route using -drawInRect:fromRect:operation:fraction:
        // TODO: Support vector graphics (we rely on NSBitmapImageRep right now for pixelsWide/pixelsHigh)
        
        id anyRep = [image bestRepresentationForDevice:nil];
        if (![anyRep isKindOfClass:[NSBitmapImageRep class]])
            throw std::logic_error("Cannot read vector graphics files");
        NSBitmapImageRep* rep = (NSBitmapImageRep*)anyRep;
        
        bitmap.resize([rep pixelsWide], [rep pixelsHigh]);
        
        // Use a temporary context to draw the NSImage to the buffer.
        
        static Gosu::CFRef<CGColorSpaceRef> colorSpace(CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB));
        Gosu::CFRef<CGContextRef> context(CGBitmapContextCreate(bitmap.data(), bitmap.width(), bitmap.height(),
                                    8, bitmap.width() * 4,
                                    colorSpace.obj(), kCGImageAlphaPremultipliedLast)); // kCGBitmapByteOrder32Host?
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:context.obj() flipped:NO]];
        [image drawInRect:NSMakeRect(0, 0, bitmap.width(), bitmap.height()) fromRect:NSZeroRect
            operation:NSCompositeCopy fraction:1.0];
        [NSGraphicsContext restoreGraphicsState];
    }
    #endif
}

Gosu::Bitmap Gosu::loadImageFile(const std::wstring& filename)
{
    ObjRef<NSString> filenameRef([[NSString alloc] initWithUTF8String: wstringToUTF8(filename).c_str()]);
    ObjRef<APPLE_IMAGE> image([[APPLE_IMAGE alloc] initWithContentsOfFile: filenameRef.obj()]);
    if (!image.get())
        throw std::runtime_error("Cannot load image file " + wstringToUTF8(filename));
    
    Bitmap bitmap;
    appleImageToBitmap(image.obj(), bitmap);
    if (boost::iends_with(filename, L".bmp"))
        applyColorKey(bitmap, Color::FUCHSIA);
    return bitmap;
}

Gosu::Bitmap Gosu::loadImageFile(Reader reader)
{
    char signature[2];
    reader.read(signature, 2);
    reader.seek(-2);
    
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
    
    std::size_t length = reader.resource().size() - reader.position();
    ObjRef<NSMutableData> buffer([[NSMutableData alloc] initWithLength: length]);
    reader.read([buffer.get() mutableBytes], length);
    
    ObjRef<APPLE_IMAGE> image([[APPLE_IMAGE alloc] initWithData: buffer.get()]);
    if (!image.get())
        throw std::runtime_error("Cannot load image file from stream");
    
    Bitmap bitmap;
    appleImageToBitmap(image.obj(), bitmap);
    if (signature[0] == 'B' && signature[1] == 'M')
        applyColorKey(bitmap, Color::FUCHSIA);
    return bitmap;
}

#ifndef GOSU_IS_IPHONE
void Gosu::saveImageFile(const Bitmap& bitmap, const std::wstring& filename)
{
    File file(filename, fmReplace);
    saveImageFile(bitmap, file.backWriter(), filename);
}

void Gosu::saveImageFile(const Bitmap& originalBitmap, Writer writer, const std::wstring& formatHint)
{
    NSBitmapImageFileType fileType;
    if (boost::iends_with(formatHint, L"png"))
        fileType = NSPNGFileType;
    else if (boost::iends_with(formatHint, L"bmp"))
        fileType = NSBMPFileType;
    else if (boost::iends_with(formatHint, L"gif"))
        fileType = NSGIFFileType;
    else if (boost::iends_with(formatHint, L"jpg") || boost::iends_with(formatHint, L"jpeg"))
        fileType = NSJPEGFileType;
    else if (boost::iends_with(formatHint, L"tif") || boost::iends_with(formatHint, L"tiff"))
        fileType = NSTIFFFileType;
    else
        throw std::runtime_error("Unsupported image format for writing: " + wstringToUTF8(formatHint));
    
    Bitmap copyOfBitmap;
    const Bitmap& bitmap = (fileType == NSBMPFileType) ? copyOfBitmap : originalBitmap;
    if (fileType == NSBMPFileType)
    {
        copyOfBitmap = originalBitmap;
        unapplyColorKey(copyOfBitmap, Color::FUCHSIA);
    }

    unsigned char* plane = (unsigned char*)bitmap.data();
    ObjRef<NSBitmapImageRep> rep([[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:&plane pixelsWide:bitmap.width() pixelsHigh:bitmap.height()
        bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO
        colorSpaceName:NSDeviceRGBColorSpace // Nobody really seems to know which one to use
        bitmapFormat:NSAlphaNonpremultipliedBitmapFormat bytesPerRow:0 bitsPerPixel:0]);

    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
    NSData* data = [rep.obj() representationUsingType:fileType properties:nil];
    writer.write([data bytes], [data length]);
}
#else
void Gosu::saveImageFile(const Bitmap& bmp, const std::wstring& filename)
{
    Buffer buffer;
    saveImageFile(bmp, buffer.backWriter());
    saveFile(buffer, filename);
}

void Gosu::saveImageFile(const Bitmap& bmp, Writer writer, const std::wstring& formatHint)
{    
    if (boost::iends_with(formatHint, L"bmp"))
    {
        Bitmap bitmap = bmp;
        unapplyColorKey(bitmap, Color::FUCHSIA);
        return Gosu::saveToBMP(bitmap, writer);
    }
    
    ObjRef<NSAutoreleasePool> pool([NSAutoreleasePool new]);
    
    CGDataProviderRef dataProvider =
        CGDataProviderCreateWithData(0, bmp.data(), bmp.width() * bmp.height() * 4, 0);
    
    static CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    
    CGImageRef imageRef =
        CGImageCreate(bmp.width(), bmp.height(), 8, 32, bmp.width() * 4, colorspace,
                      kCGImageAlphaLast, dataProvider, 0, false, kCGRenderingIntentDefault);
    
    ObjRef<UIImage> image([[UIImage alloc] initWithCGImage: imageRef]);
    
    NSData* data;
    if (boost::iends_with(formatHint, L"jpeg") || boost::iends_with(formatHint, L"jpg"))
        data = UIImageJPEGRepresentation(image.get(), 0.0);
    else if (boost::iends_with(formatHint, L"png"))
        data = UIImagePNGRepresentation(image.get());
    else
        throw std::runtime_error("Unsupported image format for writing: " + wstringToUTF8(formatHint));
    writer.write([data bytes], [data length]);
    image.reset();
    
    CGImageRelease(imageRef);
    
    CGDataProviderRelease(dataProvider);
    
    return writer;
}
#endif
