#import <Gosu/Text.hpp>
#import <Gosu/Bitmap.hpp>
#import <Gosu/Utility.hpp>
#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <map>
#import <cmath>
using namespace std;

std::wstring Gosu::defaultFontName()
{
    // OF COURSE Helvetica is better - but the dots above my capital umlauts get
    // eaten when I use it with Gosu. Until this is fixed, keep Arial. (TODO)
    return L"Arial";
}

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    // This will, of course, compute a too large size; fontHeight is in pixels, the method expects point.
    UIFont* font = [UIFont fontWithName: [NSString stringWithUTF8String: wstringToUTF8(fontName).c_str()]
                           size: fontHeight];
    CGSize size = [[NSString stringWithUTF8String: wstringToUTF8(text).c_str()]
                             sizeWithFont: font];
                           
    // Now adjust the scaling...
    return ceil(size.width / size.height * fontHeight);
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    const char* utf8FontName = wstringToUTF8(fontName).c_str();
    UIFont* font = [UIFont fontWithName: [NSString stringWithUTF8String: utf8FontName]
                           size: fontHeight];
    NSString* string = [NSString stringWithUTF8String: wstringToUTF8(text).c_str()];

    // This will, of course, compute a too large size; fontHeight is in pixels, the method expects point.
    CGSize size = [string sizeWithFont: font];
    
    unsigned width = ceil(size.width / size.height * fontHeight);

    // Get the width and height of the image
    Bitmap bmp;
    bmp.resize(width, fontHeight);
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context =
        CGBitmapContextCreate(const_cast<unsigned*>(bmp.glCompatibleData()),
                              bmp.width(), bmp.height(), 8, bmp.width() * 4,
                              colorSpace,
                              kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
    CGFloat color[] = { c.green() / 255.0, c.blue() / 255.0, c.red() / 255.0, 0 };
    CGContextSetStrokeColor(context, color);
    CGContextSetFillColor(context, color);
    
    // TODO: Should probably use CGShowText instead of all the ObjC bloat.
    
    // Use new font with proper size this time.
    font = [UIFont fontWithName: [NSString stringWithUTF8String: utf8FontName] size: fontHeight * fontHeight / size.height];
    
    CGContextTranslateCTM(context, 0, fontHeight);
    CGContextScaleCTM(context, 1, -1);
    UIGraphicsPushContext(context);
    [string drawAtPoint: CGPointZero withFont: font];
    UIGraphicsPopContext();
    CGContextRelease(context);

    // Done!
    bitmap.insert(bmp, x, y);
}
