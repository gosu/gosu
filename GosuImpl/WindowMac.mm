#include <Gosu/Window.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Input.hpp>
#include "MacUtility.hpp"
#include <Gosu/Timing.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Utility.hpp>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <memory>
#include <vector>
using namespace std::tr1::placeholders;

#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#import <Carbon/Carbon.h>

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
    
    NSRect screenRect = [[NSScreen mainScreen] frame];
    
    unsigned screenWidth()
    {
        return screenRect.size.width;
    }
    
    unsigned screenHeight()
    {
        return screenRect.size.height;
    }
    
    static NSUInteger windowStyleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask;
    
    void scaleDownWindowIfNecessary(unsigned& width, unsigned& height)
    {
        NSRect usableFrameRect = [[NSScreen mainScreen] visibleFrame];
        NSRect usableContentRect = [NSWindow contentRectForFrameRect:usableFrameRect styleMask:windowStyleMask];
        double factor = std::min(usableContentRect.size.width / width, usableContentRect.size.height / height);
        if (factor < 1) {
            width *= factor;
            height *= factor;
        }
    }
}

#ifndef __LP64__
// Workaround for Apple NSScreen bug :(
// TODO - fill in the details ... on which systems does Gosu break without this?
@interface NSScreen (NSScreenAccess)
- (void)setFrame:(NSRect)frame;
@end

@implementation NSScreen (NSScreenAccess)
- (void)setFrame:(NSRect)frame;
{
    _frame = frame;
}
@end
#endif

// Necessary to catch input events in fullscreen mode
@interface GosuApplication : NSApplication
{
    Gosu::Input* input;
}
- (void)sendEvent:(NSEvent *)anEvent;
- (void)setInput:(Gosu::Input&)ipt;
@end
@implementation GosuApplication
- (void)sendEvent:(NSEvent *)anEvent
{
    switch ([anEvent type])
    {
        case NSLeftMouseDown:
        case NSLeftMouseUp:
        case NSRightMouseDown:
        case NSRightMouseUp:
        case NSOtherMouseDown:
        case NSOtherMouseUp:
        case NSScrollWheel:
        case NSKeyUp:
        case NSKeyDown:
        case NSFlagsChanged:
            input->feedNSEvent(anEvent);
            break;
        default:
            [super sendEvent:anEvent];
    }
}
- (void)setInput:(Gosu::Input&)ipt
{
    input = &ipt;
}
@end

namespace 
{
    void cgCheck(CGDisplayErr err, const char* action)
    {
        if (err != CGDisplayNoErr)
            throw std::runtime_error(std::string("Core Graphics error while ") + action);
    }
}

typedef void (*WindowProc)(Gosu::Window&);

@interface GosuForwarder : NSObject
#ifdef __LP64__
                                    <NSWindowDelegate>
#endif
{
    Gosu::Window* win;
    WindowProc pr;
}
- (id)initWithWindow:(Gosu::Window *)window withProc:(WindowProc)proc;
- (void)doTick:(NSTimer *)timer;
- (BOOL)windowShouldClose:(id)sender;
@end

@implementation GosuForwarder
- (id)initWithWindow:(Gosu::Window*)window withProc:(WindowProc)proc
{
    if ((self = [super init])) {
        win = window;
        pr = proc;
    }
    return self;
}

- (void)doTick:(NSTimer*)timer
{
    pr(*win);
}

- (BOOL)windowShouldClose:(id)sender
{
    [NSApp stop:nil];
    return YES;
}
@end

#define OVERRIDE_METHOD(method)       \
    - (void)method:(NSEvent*) event   \
    {                                 \
        _input->feedNSEvent(event);   \
    }

@interface GosuWindow : NSWindow
{
    Gosu::Input* _input;
}
@end
@implementation GosuWindow
- (void)setInput:(Gosu::Input *)input
{
    _input = input;
}

OVERRIDE_METHOD(keyDown);
OVERRIDE_METHOD(keyUp);
OVERRIDE_METHOD(flagsChanged);
OVERRIDE_METHOD(mouseDown);
OVERRIDE_METHOD(mouseUp);
OVERRIDE_METHOD(rightMouseDown);
OVERRIDE_METHOD(rightMouseUp);
OVERRIDE_METHOD(otherMouseDown);
OVERRIDE_METHOD(otherMouseUp);
OVERRIDE_METHOD(scrollWheel);
@end

@interface GosuView : NSView
{
    Gosu::Input *_input;
}
@end
@implementation GosuView
- (void)setInput:(Gosu::Input *)input
{
    _input = input;
    
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}
OVERRIDE_METHOD(keyDown);
OVERRIDE_METHOD(keyUp);
OVERRIDE_METHOD(flagsChanged);
OVERRIDE_METHOD(mouseDown);
OVERRIDE_METHOD(mouseUp);
OVERRIDE_METHOD(rightMouseDown);
OVERRIDE_METHOD(rightMouseUp);
OVERRIDE_METHOD(scrollWheel);
@end

#undef OVERRIDE_METHOD

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 1070
@interface NSView (GosuProvideMissingMethodDeclaration)
- (NSRect)convertRectToBacking:(NSRect)aRect;
@end
#endif

struct Gosu::Window::Impl
{
    ObjRef<NSAutoreleasePool> pool;
    
    // Windowed mode: NSWindow subclass and delegate that forwards events to it.
    ObjRef<GosuWindow> window;
    ObjRef<GosuForwarder> forwarder;
    
    // Fullscreen mode. Also remember old display mode.
    CFDictionaryRef newMode, savedMode;
	NSRect savedFrame;
    
    ObjRef<NSOpenGLContext> context;
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Input> input;
    double interval;
    bool mouseViz;
    
    void createWindowAndAdjustWidthHeight(unsigned& width, unsigned& height)
    {
        NSRect rect = NSMakeRect(0, 0, width, height);
        window.reset([[GosuWindow alloc] initWithContentRect:rect styleMask:windowStyleMask 
                                         backing:NSBackingStoreBuffered defer:NO]);
        [window.obj() retain]; // TODO: Why?
        
        [window.obj() setContentView:[[GosuView alloc] init]];
        if ([[window.obj() contentView] respondsToSelector:@selector(wantsBestResolutionOpenGLSurface)])
        {
            [[window.obj() contentView] setValue:[NSNumber numberWithBool:YES] forKey:@"wantsBestResolutionOpenGLSurface"];
            NSRect pointRect = NSMakeRect(0, 0, width, height);
            NSRect backingRect = [(id)[window.obj() contentView] convertRectToBacking:pointRect];
            width = backingRect.size.width;
            height = backingRect.size.height;
        }
        [window.obj() center];
    }
    
    static void doTick(Window& window);
};

// An addition for MacRuby support:
// Without this variable, the context seems to get eaten by the GC. (?)

static NSOpenGLContext* context;

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
                     double updateInterval)
: pimpl(new Impl)
{
    if (fullscreen) {
        NSLog(@"Fullscreen mode is temporarily unavailable on OS X; see https://github.com/jlnr/gosu/issues/157");
        fullscreen = false;
    }
    
    pimpl->pool.reset([[NSAutoreleasePool alloc] init]); // <- necessary...?
    
    // Create NSApp global variable
    if (fullscreen)
        [GosuApplication sharedApplication];
    else
        [NSApplication sharedApplication];
    
    // Settings, depending on fullscreen or not
    NSOpenGLPixelFormatAttribute windowedAttrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAScreenMask,
        (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()),
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)16,
        (NSOpenGLPixelFormatAttribute)0
    };
    NSOpenGLPixelFormatAttribute fullscreenAttrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAScreenMask,
        (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()),
        NSOpenGLPFAFullScreen,
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)16,
        (NSOpenGLPixelFormatAttribute)0
    };
    NSOpenGLPixelFormatAttribute* attrs = fullscreen ? fullscreenAttrs : windowedAttrs;
    
    // Create pixel format and OpenGL context
    ObjRef<NSOpenGLPixelFormat> fmt([[NSOpenGLPixelFormat alloc] initWithAttributes:attrs]);
    if (! fmt.get())
        throw std::runtime_error("Could not find a suitable OpenGL pixel format");
    ::context = [[NSOpenGLContext alloc] initWithFormat:fmt.obj() shareContext:nil];
    pimpl->context.reset(context);
    if (! pimpl->context.get())
        throw std::runtime_error("Unable to create an OpenGL context with the supplied pixel format");
    
    unsigned realWidth = width, realHeight = height;
    unsigned pixelWidth = width, pixelHeight = height;
    
    if (fullscreen) {
        // Fullscreen: Create no window, instead change resolution.
        
        // Save old mode and retrieve BPP
        pimpl->savedMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
        pimpl->savedFrame = [[NSScreen mainScreen] frame];
        int bpp;
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(pimpl->savedMode, kCGDisplayBitsPerPixel),
                         kCFNumberIntType, &bpp);
        pimpl->newMode =
            CGDisplayBestModeForParameters(kCGDirectMainDisplay, bpp, width, height, 0);
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(pimpl->newMode, kCGDisplayWidth),
                         kCFNumberIntType, &width);
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(pimpl->newMode, kCGDisplayHeight),
                         kCFNumberIntType, &height);
        
        // Actual changing postponed until show().
    }
    else
    {
        scaleDownWindowIfNecessary(realWidth, realHeight);
        pixelWidth = realWidth;
        pixelHeight = realHeight;
        pimpl->createWindowAndAdjustWidthHeight(pixelWidth, pixelHeight);
        // Tell context to draw on this window.
        [pimpl->context.obj() setView:[pimpl->window.obj() contentView]];
    }
        
    CGLEnable((CGLContextObj)[pimpl->context.obj() CGLContextObj], kCGLCEMPEngine);
    
    [pimpl->context.obj() makeCurrentContext];
    
    pimpl->graphics.reset(new Gosu::Graphics(pixelWidth, pixelHeight, fullscreen));
    pimpl->graphics->setResolution(width, height);
    
    pimpl->input.reset(new Input(pimpl->window.get()));
    pimpl->input->onButtonDown = std::tr1::bind(&Window::buttonDown, this, _1);
    pimpl->input->onButtonUp = std::tr1::bind(&Window::buttonUp, this, _1);
    pimpl->input->setMouseFactors(1.0 * width / realWidth, 1.0 * height / realHeight);
    if (fullscreen)
        [NSApp setInput:input()];
    
    pimpl->forwarder.reset([[GosuForwarder alloc] initWithWindow:this withProc:&Impl::doTick]);
    
    if (! fullscreen)
    {
        [pimpl->window.obj() setDelegate:pimpl->forwarder.obj()];
        [pimpl->window.obj() setInput:pimpl->input.get()];
        [(GosuView*)[pimpl->window.obj() contentView] setInput:pimpl->input.get()];
        // Finally, safely hide the window.
        [pimpl->window.obj() orderOut:nil];
    }
    
    pimpl->interval = updateInterval;
    pimpl->mouseViz = true;
    
    // Clear gl error flag if it should accidentally be set. (Huh?)
    while (glGetError() != GL_NO_ERROR);
}

Gosu::Window::~Window()
{
    if (graphics().fullscreen())
    {
        CGDisplaySwitchToMode(kCGDirectMainDisplay, pimpl->savedMode);
        #ifndef __LP64__
		[[NSScreen mainScreen] setFrame:pimpl->savedFrame]; 
        #endif
        CGReleaseAllDisplays();
    }
}

std::wstring Gosu::Window::caption() const
{
    ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
    const char* utf8 = [[pimpl->window.get() title] UTF8String];
    return utf8 ? Gosu::utf8ToWstring(utf8) : std::wstring();
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    std::string utf8 = wstringToUTF8(caption);
    ObjRef<NSString> title([[NSString alloc] initWithUTF8String:utf8.c_str()]);
    [pimpl->window.get() setTitle:title.obj()];
}

double Gosu::Window::updateInterval() const
{
    return pimpl->interval;
}

void Gosu::Window::show()
{
    // This is for Ruby/Gosu and misc. hackery:
    // Usually, applications on the Mac can only get keyboard and mouse input if
    // run by double-clicking an .app. So if this is run from the Terminal (i.e.
    // during Ruby/Gosu game development), tell the OS we need input in any case.
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcess(&psn);

    [NSThread setThreadPriority:1.0];
 
    if (graphics().fullscreen())
    {
        // Set new mode
        cgCheck(CGDisplayCapture(kCGDirectMainDisplay), "capturing display");
        cgCheck(CGDisplaySwitchToMode(kCGDirectMainDisplay, pimpl->newMode), "switching mode");
        
        // Start drawing in fullscreen
        [pimpl->context.obj() setFullScreen];
    }
    else
        [pimpl->window.obj() makeKeyAndOrderFront:nil];

    NSTimer* timer = [NSTimer scheduledTimerWithTimeInterval:pimpl->interval / 1000.0
                            target:pimpl->forwarder.obj() selector:@selector(doTick:)
                            userInfo:nil repeats:YES];
    [NSApp run];
    [timer invalidate];
    
    if (graphics().fullscreen())
    {
        // Resetting the mode shouldn't be all too important according to the docs.
        // Let's leave it in until time for testing comes, though.
        CGDisplaySwitchToMode(kCGDirectMainDisplay, pimpl->savedMode);
        CGDisplayRelease(kCGDirectMainDisplay);
    }
    else
    {
        [pimpl->window.obj() orderOut:nil];
    }
}

void Gosu::Window::close()
{
    [NSApp stop:nil];
    
    // NSApp doesn't check its 'stopped' flag until it finishes processing
    // the next event (timers are not events), so here's a rather hacky way
    // to make sure it has one in its queue
    [NSApp postEvent:[NSEvent otherEventWithType:NSApplicationDefined
                              location:NSZeroPoint
                              modifierFlags:0
                              timestamp:0.0
                              windowNumber:0
                              context:NULL
                              subtype:0
                              data1:0
                              data2:0] atStart:NO];
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *pimpl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *pimpl->input;
}

namespace
{
    void makeCurrentContext(NSOpenGLContext *context)
    {
        [context makeCurrentContext];
    }
    
    void releaseContextAndDelete(NSOpenGLContext *context, std::tr1::function<void()> *function)
    {
        [context release];
        delete function;
    }
}

Gosu::Window::SharedContext Gosu::Window::createSharedContext()
{
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAScreenMask,
        (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()),
        NSOpenGLPFAFullScreen,
        NSOpenGLPFANoRecovery,
        (NSOpenGLPixelFormatAttribute)0
    };
    
    NSOpenGLPixelFormat* pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];

    NSOpenGLContext* ctx = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:pimpl->context.obj()];
    
    // TODO: Exception safety? Oh man, is this construction really worth it? :)
    std::tr1::function<void()> *makeCurrent =
        new std::tr1::function<void()>(std::tr1::bind(makeCurrentContext, ctx));
    return SharedContext(makeCurrent,
        std::tr1::bind(releaseContextAndDelete, ctx, _1));
}

namespace GosusDarkSide
{
    // TODO: Find a way for this to fit into Gosu's design.
    // This can point to a function that wants to be called every
    // frame, e.g. rb_thread_schedule.
    typedef void (*HookOfHorror)();
    HookOfHorror oncePerTick = 0;
}

void Gosu::Window::Impl::doTick(Window& window)
{
    // Enable vsync.
    GLint value = 1;
    [window.pimpl->context.obj() setValues:&value forParameter:NSOpenGLCPSwapInterval];
    
    if ((window.graphics().fullscreen() ||
        NSPointInRect([window.pimpl->window.obj() mouseLocationOutsideOfEventStream],
                      [[window.pimpl->window.obj() contentView] frame])) &&
        [NSApp isActive] && !window.needsCursor())
    {
        if (window.pimpl->mouseViz)
            [NSCursor hide];
        window.pimpl->mouseViz = false;
    }
    else
    {
        if (! window.pimpl->mouseViz)
            [NSCursor unhide];
        window.pimpl->mouseViz = true;
    }
    
    Gosu::Song::update();
    window.input().update();
    window.update();

    if (window.needsRedraw() and window.graphics().begin())
    {
        FPS::registerFrame();
        window.draw();
        window.graphics().end();
        [window.pimpl->context.obj() flushBuffer];
    }
    
    if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
}

// Deprecated.

class Gosu::Audio {};
namespace { Gosu::Audio dummyAudio; }

const Gosu::Audio& Gosu::Window::audio() const
{
    return dummyAudio;
}
 
Gosu::Audio& Gosu::Window::audio()
{
    return dummyAudio;
}
 
