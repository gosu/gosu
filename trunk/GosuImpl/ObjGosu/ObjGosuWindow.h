#import <Cocoa/Cocoa.h>

#include <Gosu/Window.hpp>

@interface ObjGosuWindow : NSObject {
    Gosu::Window* window;
}
- (id) initWithWidth:(int)width height:(int)height inFullscreen:(BOOL)fullscreen;
- (void) show;
@end
