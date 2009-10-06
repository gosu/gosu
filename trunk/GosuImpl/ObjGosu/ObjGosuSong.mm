#import <Cocoa/Cocoa.h>
#import <Gosu/Audio.hpp>
#import <Gosu/Utility.hpp>
#import <Gosu/Window.hpp>

@interface ObjGosuSong : NSObject {
    Gosu::Song* _song;
}
@end

@implementation ObjGosuSong
- (id)initWithWindow:(id)window filename:(NSString*)filename
{
    if (not (self = [super init]))
        return NULL;
    
    _song = new Gosu::Song(((Gosu::Window*)[[window _objcObject] _cppObject])->audio(),
        Gosu::widen([filename UTF8String]));
    
    return self;
}

- (void)playWithLoop: (BOOL)looping
{
    _song->play(looping);
}

- (void)stop
{
    _song->stop();
}

- (float)volume
{
    _song->volume();
}

- (void)setVolume:(float)volume
{
    _song->changeVolume(volume);
}

@end
