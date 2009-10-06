#import "ObjGosuImage.h"
#import "ObjGosuWindow.h"
#import <Gosu/Image.hpp>
#import <Gosu/Utility.hpp>
#import <Gosu/Window.hpp>
#import <vector>

@implementation ObjGosuImage
- (int)width
{
    return _image->width();
}

- (int)height
{
    return _image->height();
}

- (id)initWithWindow:(id)window filename:(NSString*)filename tileable:(BOOL)tileable
{
    if (not (self = [super init]))
        return NULL;
    
    _image = new Gosu::Image(((Gosu::Window*)[[window _objcObject] _cppObject])->graphics(),
        Gosu::widen([filename UTF8String]), tileable);
    
    return self;
}

- (id)initWithCPPObject:(Gosu::Image*)image
{
    if (not (self = [super init]))
        return NULL;
    
    _image = image;
    
    return self;
}

+ (NSMutableArray*)loadTilesWithWindow:(id)window filename:(NSString*)filename
    tileWidth:(int)tileWidth tileHeight:(int)tileHeight tileable:(BOOL)tileable
{
    std::vector<Gosu::Image*> images;
    imagesFromTiledBitmap(((Gosu::Window*)[[window _objcObject] _cppObject])->graphics(),
        Gosu::utf8ToWstring([filename UTF8String]), tileWidth, tileHeight, tileable, images);

    NSMutableArray* array = [NSMutableArray arrayWithCapacity:images.size()];
    for (int i = 0; i < images.size(); ++i)
        [array insertObject:[[ObjGosuImage alloc] initWithCPPObject:images[i]] atIndex:i];
    return array;
}

- (void)drawWithX: (float)x y:(float)y z:(float)z
    factorX:(float)factorX factorY:(float)factorY color:(unsigned long)color
{
    _image->draw(x, y, z, factorX, factorY, color);
}

- (void)drawRotWithX: (float)x y:(float)y z:(float)z angle:(float)angle
    centerX:(float)centerX centerY:(float)centerY
    factorX:(float)factorX factorY:(float)factorY color:(unsigned long)color
{
    _image->drawRot(x, y, z, angle, centerX, centerY, factorX, factorY, color);
}
@end
