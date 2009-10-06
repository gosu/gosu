#import "ObjGosuWindow.h"
#import <Gosu/Graphics.hpp>
#import <Gosu/Input.hpp>
#import <Gosu/Utility.hpp>
#import <Gosu/Window.hpp>
#import <MacRuby/MacRuby.h>

class _ObjGosuInternalWindow : public Gosu::Window
{
    id delegate;

public:
    _ObjGosuInternalWindow(id delegate, unsigned width, unsigned height, 
        bool fullscreen, double updateInterval)
    : Gosu::Window(width, height, fullscreen, updateInterval), delegate(delegate)
    {
    }
    
    void update()
    {
        [delegate update];
    }
    
    void draw()
    {
        [delegate draw];
    }
    
    void buttonUp(Gosu::Button btn)
    {
        [delegate buttonUp: [NSNumber numberWithUnsignedInt: btn.getId()]];
    }
    
    void buttonDown(Gosu::Button btn)
    {
        [delegate buttonDown: [NSNumber numberWithUnsignedInt: btn.getId()]];
    }
    
    BOOL needsRedraw()
    {
        return [delegate needsRedraw] != nil;
    }
};

@implementation ObjGosuWindow
- (Gosu::Window*)_cppObject
{
    return _window;
}

- (NSString*)caption
{
    return nil; // TODO_window->caption();
}

- (void)setCaption: (NSString*) caption
{
    _window->setCaption(Gosu::utf8ToWstring([caption UTF8String]));
}

- (unsigned)height
{
    return _window->graphics().height();
}

- (float)mouseX
{
    return _window->input().mouseX();
}

- (void)setMouseX: (float)mouseX
{
    _window->input().setMousePosition(mouseX, _window->input().mouseY());
}

- (float)mouseY
{
    return _window->input().mouseY();
}

- (void)setMouseY: (float)mouseY
{
    _window->input().setMousePosition(_window->input().mouseX(), mouseY);
}

- (id)textInput
{
    // TODO
    return nil;
}

- (void)setTextInput: (id)textInput
{
    // TODO
}

- (float)updateInterval
{
    return _window->updateInterval();
}

- (unsigned)width
{
    return _window->graphics().width();
}

- (id)initWithDelegate: (id)delegate width:(int)width height:(int)height inFullscreen:(BOOL)fullscreen updateInterval:(float)updateInterval
{
    if (not (self = [super init]))
        return NULL;
    _window = new _ObjGosuInternalWindow(delegate, width, height, fullscreen, updateInterval);
    return self;
}

- (void)show
{
    _window->show();
}

- (void)close
{
    _window->close();
}

- (void)update
{
}

- (void)draw
{
}

- (NSNumber*)isButtonDown: (unsigned)button
{
    return [NSNumber numberWithBool: _window->input().down(Gosu::Button(button))];
}
@end

extern "C" void Init_gosu()
{
//    [[MacRuby sharedRuntime] evaluateString:
//        @"module Gosu\n"
//        @"  class Window < ::ObjGosuWindow\n"
//        @"    def initialize(width, height, fullscreen, update_interval)\n"
//        @"      initWithWidth width, height: height, inFullscreen: fullscreen, updateInterval: update_interval\n"
//        @"    end\n"
//        @"    def mouse_x; mouseX; end\n"
//        @"    def mouse_y; mouseY; end\n"
//        @"    def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode)\n"
//        @"      drawQuadAtX1(x1, y1: y1, color1: c1, x2: x2, y2: y2, color2: c2, x3: x3, y3: y3, color3: c3, x4: x4, y4: y4, color4: c4, z: z, mode: mode)\n"
//        @"    end\n"
//        @"  end\n"
//        @"end\n"
//    ];
}
