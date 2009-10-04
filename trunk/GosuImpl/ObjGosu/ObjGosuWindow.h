#import <Cocoa/Cocoa.h>
#import <Gosu/Fwd.hpp>

class _ObjGosuInternalWindow;

@interface ObjGosuWindow : NSObject {
    _ObjGosuInternalWindow* _window;
}
- (Gosu::Window*)_cppObject;

- (NSString*) caption;
- (void) setCaption: (NSString*) caption;
- (unsigned) height;
- (float) mouseX;
- (void) setMouseX: (float)mouseX;
- (float) mouseY;
- (void) setMouseY: (float)mouseY;
- (id) textInput;
- (void) setTextInput: (id)textInput;
- (float) updateInterval;
- (unsigned) width;

- (id) initWithDelegate:(id)delegate width:(int)width height:(int)height
    inFullscreen:(BOOL)fullscreen updateInterval:(float)updateInterval;
- (void) show;

- (void) drawQuadAtX1: (float)x1 y1: (float)y1 color1:(unsigned)c1
                   x2: (float)x2 y2: (float)y2 color2:(unsigned)c2
                   x3: (float)x3 y3: (float)y3 color3:(unsigned)c3
                   x4: (float)x4 y4: (float)y4 color4:(unsigned)c4
                   z: (float)z mode: (int)mode;
@end
