#import <Cocoa/Cocoa.h>
#import <Gosu/Audio.hpp>
#import <Gosu/Utility.hpp>
#import <Gosu/Window.hpp>

@interface ObjGosuSample : NSObject {
    Gosu::Sample* _sample;
}
@end

@implementation ObjGosuSample
- (id)initWithWindow:(id)window filename:(NSString*)filename
{
    if (not (self = [super init]))
        return NULL;
    
    _sample = new Gosu::Sample(((Gosu::Window*)[[window _objcObject] _cppObject])->audio(),
        Gosu::widen([filename UTF8String]));
    
    return self;
}

- (void)playWithVolume: (float)volume speed:(float)speed looping:(BOOL)looping
{
    _sample->play(volume, speed, looping);
}

- (void)playWithPan: (float)pan volume: (float)volume speed:(float)speed looping:(BOOL)looping
{
    _sample->playPan(pan, volume, speed, looping);
}
@end
