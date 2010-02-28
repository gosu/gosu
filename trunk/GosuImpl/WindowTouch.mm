#import <Gosu/Window.hpp>
#import <Gosu/Graphics.hpp>
#import <Gosu/Audio.hpp>
#import <Gosu/Input.hpp>
#import <GosuImpl/MacUtility.hpp>   
#import <GosuImpl/Graphics/GosuView.hpp>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

namespace Gosu {
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    
    unsigned screenWidth()
    {
        return screenRect.size.width;
    }
    
    unsigned screenHeight()
    {
        return screenRect.size.height;
    }
}

int main(int argc, char *argv[]) {
    Gosu::ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
	return UIApplicationMain(argc, argv, nil, @"GosuAppDelegate");
}

class Gosu::Audio {};

struct Gosu::Window::Impl {
    ObjRef<NSAutoreleasePool> pool;
    ObjRef<UIWindow> window;
    ObjRef<GosuView> view;
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Audio> audio;
    boost::scoped_ptr<Input> input;
    double interval;
};

Gosu::Window& windowInstance();

@interface GosuAppDelegate : NSObject <UIApplicationDelegate> {
}
@end

// Ugly monkey patching to bridge the C++ and ObjC sides.
namespace
{
    GosuView* gosuView = nil;
}

@implementation GosuAppDelegate
- (void)applicationDidFinishLaunching:(UIApplication *)application {
    [[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    [UIApplication sharedApplication].statusBarOrientation = UIInterfaceOrientationLandscapeRight;
    
    windowInstance();
    
    [[NSTimer scheduledTimerWithTimeInterval: windowInstance().updateInterval() / 1000.0
              target: self
              selector: @selector(doTick:)
              userInfo: nil
              repeats: YES] retain];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	// TODO: stop updating; periodically draw
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	// TODO: start updating again
}

- (void)doTick:(NSTimer*)timer {
    windowInstance().update();
    [gosuView drawView];
    [gosuView removeDeadTouches];
}
@end

Gosu::Window::Window(unsigned width, unsigned height,
    bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
    pimpl->pool.reset([[NSAutoreleasePool alloc] init]);
	pimpl->window.reset([[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]);
    pimpl->view.reset([[GosuView alloc] initWithFrame:[pimpl->window.obj() bounds]]);
    gosuView = pimpl->view.obj();

	[pimpl->window.obj() addSubview: pimpl->view.obj()];
    
    pimpl->graphics.reset(new Graphics(320, 480, false));
    pimpl->graphics->setResolution(480, 320);
    pimpl->audio.reset(new Audio());
    pimpl->input.reset(new Input());
    pimpl->interval = updateInterval;

    [pimpl->window.obj() makeKeyAndVisible];
}

Gosu::Window::~Window()
{
}

std::wstring Gosu::Window::caption() const {
    return L"";
}

void Gosu::Window::setCaption(const std::wstring& caption) {
}

double Gosu::Window::updateInterval() const
{
    return pimpl->interval;
}

const Gosu::Graphics& Gosu::Window::graphics() const {
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics() {
    return *pimpl->graphics;
}

const Gosu::Audio& Gosu::Window::audio() const {
    return *pimpl->audio;
}

Gosu::Audio& Gosu::Window::audio() {
    return *pimpl->audio;
}

const Gosu::Input& Gosu::Window::input() const {
    return *pimpl->input;
}

Gosu::Input& Gosu::Window::input() {
    return *pimpl->input;
}

void Gosu::Window::show()
{
    throw std::logic_error("Gosu::Window::show not available on iPhone");
}

void Gosu::Window::close()
{
    throw "NYI";
}

const Gosu::Touches& Gosu::Window::currentTouches() const
{
    return [pimpl->view.obj() currentTouches];
}
