#import <Gosu/Window.hpp>
#import <Gosu/Audio.hpp>
#import <Gosu/Graphics.hpp>
#import <Gosu/Input.hpp>
#import <GosuImpl/MacUtility.hpp>
#import <Gosu/Timing.hpp>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <boost/bind.hpp>
#import <vector>

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
{
    Gosu::Window* win;
    WindowProc pr;
}
- (id)initWithWindow: (Gosu::Window*)window withProc:(WindowProc)proc;
- (void)doTick: (NSTimer*)timer;
- (BOOL)windowShouldClose: (id)sender;
@end

@implementation GosuForwarder
- (id)initWithWindow: (Gosu::Window*)window withProc:(WindowProc)proc
{
    if (![super init])
        return nil;
    win = window;
    pr = proc;
    return self;
}

- (void)doTick: (NSTimer*)timer
{
    pr(*win);
}

- (BOOL)windowShouldClose: (id)sender
{
    [NSApp stop: nil];
    return YES;
}
@end

#define OVERRIDE_METHOD(method)       \
    - (void) method: (NSEvent*) event \
    {                                 \
        _input->feedNSEvent(event);   \
    }

@interface GosuWindow : NSWindow
{
    Gosu::Input* _input;
}
@end
@implementation GosuWindow
- (void) setInput: (Gosu::Input*)input
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
OVERRIDE_METHOD(scrollWheel);
@end

@interface GosuView : NSView
{
    Gosu::Input* _input;
}
@end
@implementation GosuView
- (void) setInput: (Gosu::Input*)input
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
OVERRIDE_METHOD(scrollWheel);
@end

#undef OVERRIDE_METHOD

struct Gosu::Window::Impl
{
    ObjRef<NSAutoreleasePool> pool;
    
    // Windowed mode: NSWindow subclass and delegate that forwards events to it.
    ObjRef<GosuWindow> window;
    ObjRef<GosuForwarder> forwarder;
    
    // Fullscreen mode. Also remember old display mode.
    CFDictionaryRef newMode, savedMode;
    
    ObjRef<NSOpenGLContext> context;
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Audio> audio;
    boost::scoped_ptr<Input> input;
    double interval;
    bool mouseViz;
    
    void createWindow(unsigned width, unsigned height)
    {
        NSRect rect = NSMakeRect(0, 0, width, height);
        unsigned style = NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask;
        window.reset([[GosuWindow alloc] initWithContentRect: rect styleMask:style 
                                                            backing:NSBackingStoreBuffered defer:NO]);
        [window.obj() retain]; // ...or is it autorelease?
        
        [window.obj() setContentView: [[GosuView alloc] init]];

        [window.obj() center];
        [window.obj() makeKeyAndOrderFront:nil];
    }
    
    static void doTick(Window& window);
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
                     double updateInterval)
: pimpl(new Impl)
{
    pimpl->pool.reset([[NSAutoreleasePool alloc] init]); // <- necessary...?
    
    // Create NSApp global variable
    if (fullscreen)
        [GosuApplication sharedApplication];
    else
        [NSApplication sharedApplication];
    
    // Settings, depending on fullscreen or not
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAScreenMask,
        (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()),
        NSOpenGLPFAFullScreen,
        NSOpenGLPFADepthSize,
        (NSOpenGLPixelFormatAttribute)16,
        (NSOpenGLPixelFormatAttribute)0
    };
    // Create pixel format and OpenGL context
    ObjRef<NSOpenGLPixelFormat> fmt([[NSOpenGLPixelFormat alloc] initWithAttributes:attrs]);
    assert(fmt.get() != nil);
    pimpl->context.reset([[NSOpenGLContext alloc] initWithFormat: fmt.obj() shareContext:nil]);
    assert(pimpl->context.get() != nil);
    
    if (fullscreen) {
        // Fullscreen: Create no window, instead change resolution.

        // Save old mode and retrieve BPP
        pimpl->savedMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
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
        // Windowed: Create window large enough to display stuff
        // TODO: Shrink resolution when screen too small
        pimpl->createWindow(width, height);
        
        // Tell context to draw on this window.
        [pimpl->context.obj() setView:[pimpl->window.obj() contentView]];
    }
    
    CGLEnable((CGLContextObj)[pimpl->context.obj() CGLContextObj], kCGLCEMPEngine);
    
    [pimpl->context.obj() makeCurrentContext];
    
    pimpl->graphics.reset(new Gosu::Graphics(width, height, fullscreen));
    
    pimpl->input.reset(new Input(pimpl->window.get()));
    pimpl->input->onButtonDown = boost::bind(&Window::buttonDown, this, _1);
    pimpl->input->onButtonUp = boost::bind(&Window::buttonUp, this, _1);
    if (fullscreen)
        [NSApp setInput: input()];
    
    pimpl->forwarder.reset([[GosuForwarder alloc] initWithWindow: this withProc: &Impl::doTick]);
    
    if (!fullscreen)
    {
        [pimpl->window.obj() setDelegate: pimpl->forwarder.obj()];
        [pimpl->window.obj() setInput: pimpl->input.get()];
        [(GosuView*)[pimpl->window.obj() contentView] setInput: pimpl->input.get()];
    }
    
    pimpl->interval = updateInterval;
    pimpl->mouseViz = true;
}

Gosu::Window::~Window()
{
}

std::wstring Gosu::Window::caption() const
{
    if (!pimpl->window.get())
        return L"";
    
    NSString* str = [pimpl->window.obj() title];
    std::vector<unichar> unibuf([str length]);
    [str getCharacters: &unibuf[0]];
    return std::wstring(unibuf.begin(), unibuf.end());
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    if (!pimpl->window.get())
        return;
    
    // This truncates the values... why on earth does a compiler use 32bit wchar_ts
    // on an UTF16 based system?
    std::vector<unichar> unibuf(caption.begin(), caption.end());
    [pimpl->window.obj() setTitle:
        [NSString stringWithCharacters: &unibuf[0] length: unibuf.size()]];
}

void Gosu::Window::show()
{
    [NSThread setThreadPriority: 1.0];
 
    if (graphics().fullscreen())
    {
        [NSCursor hide];

        // Set new mode
        cgCheck(CGDisplayCapture(kCGDirectMainDisplay), "capturing display");
        cgCheck(CGDisplaySwitchToMode(kCGDirectMainDisplay, pimpl->newMode), "switching mode");
        
        // Start drawing in fullscreen
        [pimpl->context.obj() setFullScreen];
    }

    NSTimer* timer = [NSTimer scheduledTimerWithTimeInterval: pimpl->interval / 1000.0
                            target:pimpl->forwarder.obj() selector:@selector(doTick:)
                            userInfo:nil repeats:YES];
    [NSApp run];
    [timer invalidate];
    if (graphics().fullscreen())
        [NSCursor unhide];

    if (graphics().fullscreen())
    {
        // Resetting the mod shouldn't be all too important according to the docs.
        // Let's leave it in until time for testing comes, though.
        CGDisplaySwitchToMode(kCGDirectMainDisplay, pimpl->savedMode);
        CGDisplayRelease(kCGDirectMainDisplay);
    }
}

void Gosu::Window::close()
{
    [NSApp stop:nil];
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
}

const Gosu::Audio& Gosu::Window::audio() const
{
    if (!pimpl->audio)
        pimpl->audio.reset(new Gosu::Audio);
    return *pimpl->audio;
}
 
Gosu::Audio& Gosu::Window::audio()
{
    if (!pimpl->audio)
        pimpl->audio.reset(new Gosu::Audio);
    return *pimpl->audio;
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
    void makeCurrentContext(NSOpenGLContext* context)
    {
        [(NSOpenGLContext*)context makeCurrentContext];
    }
    
    void releaseContext(NSOpenGLContext* context)
    {
        [(NSOpenGLContext*)context release];
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
    
    NSOpenGLPixelFormat* pf =
        [[[NSOpenGLPixelFormat alloc]
            initWithAttributes:attributes] autorelease];

    NSOpenGLContext* ctx = [[NSOpenGLContext alloc]
            initWithFormat: pf
            shareContext: pimpl->context.obj()];
    
    return SharedContext(new boost::function<void()>(boost::bind(makeCurrentContext, ctx)),
        boost::bind(releaseContext, ctx));
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
    GLint value = 1;
    [window.pimpl->context.obj() setValues: &value forParameter: NSOpenGLCPSwapInterval];
    
    if (!window.graphics().fullscreen())
    {
        if (NSPointInRect([window.pimpl->window.obj() mouseLocationOutsideOfEventStream],
                          [[window.pimpl->window.obj() contentView] frame]))
        {
            if (window.pimpl->mouseViz)
                [NSCursor hide];
            window.pimpl->mouseViz = false;
        }
        else
        {
            if (!window.pimpl->mouseViz)
                [NSCursor unhide];
            window.pimpl->mouseViz = true;
        }
    }
    
    window.input().update();
    window.update();
    if (window.graphics().begin())
    {
        window.draw();
        window.graphics().end();
        [window.pimpl->context.obj() flushBuffer];
    }
    
    if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
}
