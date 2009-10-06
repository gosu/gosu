#import <Cocoa/Cocoa.h>
#import <Gosu/Math.hpp>
#import <Gosu/Text.hpp>
#import <Gosu/Timing.hpp>
#import <Gosu/Utility.hpp>

@interface ObjGosu : NSObject {
}
@end

@implementation ObjGosu
+ (float)offsetXWithAngle:(float)angle distance:(float)dist
{
    return Gosu::offsetX(angle, dist);
}

+ (float)offsetYWithAngle:(float)angle distance:(float)dist
{
    return Gosu::offsetY(angle, dist);
}

+ (float)distanceFromX:(float)x1 y:(float)y1 toX:(float)x2 y:(float)y2
{
    return Gosu::distance(x1, y1, x2, y2);
}

+ (unsigned)milliseconds
{
    return Gosu::milliseconds();
}

+ (NSString*)defaultFontName
{
    return [NSString stringWithUTF8String: Gosu::wstringToUTF8(Gosu::defaultFontName()).c_str()];
}
@end
