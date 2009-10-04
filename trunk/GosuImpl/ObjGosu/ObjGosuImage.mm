#import "ObjGosuImage.h"
#import "ObjGosuWindow.h"
#import <Gosu/Image.hpp>
#import <Gosu/Utility.hpp>
#import <Gosu/Window.hpp>

@implementation ObjGosuImage
- (int)width
{
    return _image->width();
}

- (int)height
{
    return _image->height();
}

- (id) initWithWindow:(id)window filename:(NSString*)filename tileable:(BOOL)tileable
{
    if (not (self = [super init]))
        return NULL;
    
    _image = new Gosu::Image(((Gosu::Window*)[[window _objcObject] _cppObject])->graphics(),
        Gosu::widen([filename UTF8String]), tileable);
    
    return self;
}

- (void) drawWithX: (float)x y:(float)y z:(float)z
    factorX:(float)factorX factorY:(float)factorY color:(unsigned long)color
{
    _image->draw(x, y, z, factorX, factorY, color);
}
@end
