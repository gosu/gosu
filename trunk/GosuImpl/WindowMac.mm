#import <Gosu/Window.hpp>
#import <Gosu/Audio.hpp>
#import <Gosu/Graphics.hpp>
#import <Gosu/Input.hpp>
#import <Gosu/MacUtility.hpp>
#import <Gosu/Timing.hpp>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
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
        case NSRightMouseUp:
        case NSRightMouseDown:
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

#define OVERRIDE_METHOD(method)       \
    - (void) method: (NSEvent*) event \
    {                                 \
        _input->feedNSEvent(event);   \
    }

OVERRIDE_METHOD(keyDown);
OVERRIDE_METHOD(keyUp);
OVERRIDE_METHOD(flagsChanged);
OVERRIDE_METHOD(mouseDown);
OVERRIDE_METHOD(mouseUp);
OVERRIDE_METHOD(rightMouseDown);
OVERRIDE_METHOD(rightMouseUp);

#undef OVERRIDE_METHOD
@end

struct Gosu::Window::Impl
{
    ObjRef<NSAutoreleasePool> pool;
    
    // Windowed mode: NSWindow subclass and delegate that forwards events to it.
    ObjRef<GosuWindow> window;
    ObjRef<GosuForwarder> forwarder;
    
    // Fullscreen mode: Remember old display mode.
    CFDictionaryRef savedMode;
    
    ObjRef<NSOpenGLContext> context;
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Audio> audio;
    boost::scoped_ptr<Input> input;
    unsigned interval;
    bool mouseViz;
    
    void createWindow(unsigned width, unsigned height)
    {
        NSRect rect = NSMakeRect(0, 0, width, height);
        unsigned style = NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask;
        window.reset([[GosuWindow alloc] initWithContentRect: rect styleMask:style 
                                                            backing:NSBackingStoreBuffered defer:NO]);
        [window.obj() retain]; // ...or is it autorelease?
        
        [window.obj() center];
        [window.obj() makeKeyAndOrderFront:nil];
    }
    
    static void doTick(Window& window)
    {
        assert(&window);
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
    }
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
                     unsigned updateInterval)
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
        // Stop here in windowed mode:
        fullscreen ? NSOpenGLPFAScreenMask : (NSOpenGLPixelFormatAttribute)0,
        (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID()),
        NSOpenGLPFAFullScreen,
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
        
        // Choose new mode
        CFDictionaryRef newMode =
            CGDisplayBestModeForParameters(kCGDirectMainDisplay, bpp, width, height, 0);
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(newMode, kCGDisplayWidth),
                         kCFNumberIntType, &width);
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(newMode, kCGDisplayHeight),
                         kCFNumberIntType, &height);
        cgCheck(CGDisplayCapture(kCGDirectMainDisplay), "capturing display");
        cgCheck(CGDisplaySwitchToMode(kCGDirectMainDisplay, newMode), "switching mode");
        
        // Start drawing in fullscreen
        [pimpl->context.obj() setFullScreen];
    }
    else
    {
        // Windowed: Create window large enough to display stuff
        // TODO: Shrink resolution when screen too small
        pimpl->createWindow(width, height);
        
        // Tell context to draw on this window.
        [pimpl->context.obj() setView:[pimpl->window.obj() contentView]];
    }
    [pimpl->context.obj() makeCurrentContext];
    
    pimpl->graphics.reset(new Gosu::Graphics(*DisplayMode::find(width, height, fullscreen)));
    
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
    }
    
    pimpl->interval = updateInterval;
    pimpl->mouseViz = true;
}

Gosu::Window::~Window()
{
    if (graphics().fullscreen())
    {
        CGDisplaySwitchToMode(kCGDirectMainDisplay, pimpl->savedMode);
        CGReleaseAllDisplays();
    }
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
	// This is for Ruby/Gosu and misc. hackery:
	// Usually, applications on the Mac only can get keyboard and mouse input if
	// run by double-clicking an .app. So if this is run from the Terminal (i.e.
	// during Ruby/Gosu game development), tell the OS we need input in any case.
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);

    if (graphics().fullscreen())
        [NSCursor hide];
    NSTimer* timer = [NSTimer scheduledTimerWithTimeInterval: pimpl->interval / 1000.0
                            target:pimpl->forwarder.obj() selector:@selector(doTick:)
                            userInfo:nil repeats:YES];
    [NSApp run];
    [timer invalidate];
    if (graphics().fullscreen())
        [NSCursor unhide];
}

void Gosu::Window::close()
{
    [NSApp terminate:nil];
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
