#import <Cocoa/Cocoa.h>
#import <Gosu/Color.hpp>

@interface ObjGosuColor : NSObject {
    boost::uint32_t rep;
}
@end

@implementation ObjGosuColor
- (id)initWithARGB:(unsigned)argb
{
    if (not (self = [super init]))
        return NULL;
    
    rep = argb;
    
    return self;
}

- (id)initWithAlpha:(boost::uint8_t)alpha red:(boost::uint8_t)red
    green:(boost::uint8_t)green blue:(boost::uint8_t)blue
{
    if (not (self = [super init]))
        return NULL;
    
    rep = Gosu::Color(alpha, red, green, blue).argb();
    
    return self;
}

- (boost::uint8_t)alpha
{
    return Gosu::Color(rep).alpha();
}

- (void)setAlpha:(boost::uint8_t)alpha
{
    Gosu::Color temp(rep);
    temp.setAlpha(alpha);
    rep = temp.argb();
}

- (boost::uint8_t)red
{
    return Gosu::Color(rep).red();
}

- (void)setRed:(boost::uint8_t)red
{
    Gosu::Color temp(rep);
    temp.setRed(red);
    rep = temp.argb();
}

- (boost::uint8_t)green
{
    return Gosu::Color(rep).green();
}

- (void)setGreen:(boost::uint8_t)green
{
    Gosu::Color temp(rep);
    temp.setGreen(green);
    rep = temp.argb();
}

- (boost::uint8_t)blue
{
    return Gosu::Color(rep).blue();
}

- (void)setBlue:(boost::uint8_t)blue
{
    Gosu::Color temp(rep);
    temp.setBlue(blue);
    rep = temp.argb();
}

- (boost::uint32_t)to_i
{
    return rep;
}

@end
