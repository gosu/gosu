#import <Cocoa/Cocoa.h>
#import <Gosu/Font.hpp>
#import <Gosu/Utility.hpp>
#import <Gosu/Window.hpp>

@interface ObjGosuFont : NSObject {
    Gosu::Font* _font;
}
@end

@implementation ObjGosuFont
- (int)height
{
    return _font->height();
}

- (NSString*)name
{
    return [NSString stringWithUTF8String:Gosu::wstringToUTF8(_font->name()).c_str()];
}

- (id)initWithWindow: (id)window fontName: (NSString*)fontName height: (int)height
{
    if (not (self = [super init]))
        return NULL;
    
    _font = new Gosu::Font(((Gosu::Window*)[[window _objcObject] _cppObject])->graphics(),
        Gosu::widen([fontName UTF8String]), height);
    
    return self;
}

/*- (float)widthOfText: (NSString*)text
{
}*/

- (void)drawText: (NSString*)text atX:(float)x y:(float)y z:(float)z
    factorX:(float)factorX factorY:(float)factorY color:(unsigned int)color
{
    _font->draw(Gosu::widen([text UTF8String]), x, y, z, factorX, factorY, color);
}
@end
