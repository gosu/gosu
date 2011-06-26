#include <Gosu/Window.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/Input.hpp>
#include <GosuImpl/MacUtility.hpp>   
#include <GosuImpl/Graphics/GosuView.hpp>

#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

using namespace std::tr1::placeholders;

namespace Gosu
{
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
    [[NSAutoreleasePool alloc] init];
	return UIApplicationMain(argc, argv, nil, @"GosuAppDelegate");
}

class Gosu::Audio {};

struct Gosu::Window::Impl {
    ObjRef<UIWindow> window;
    ObjRef<GosuViewController> controller;
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Audio> audio;
    std::auto_ptr<Input> input;
    double interval;
};

Gosu::Window& windowInstance();

@interface GosuAppDelegate : NSObject <UIApplicationDelegate>
@end

namespace
{
    // Ugly patching to bridge the C++ and ObjC sides.
    GosuView* gosuView = nil;
    bool pausedSong = false;
    bool paused = false;
}

@implementation GosuAppDelegate
// Required according to docs...
- (void)applicationProtectedDataWillBecomeUnavailable:(UIApplication *)application
{
}

// Required according to docs...
- (void)applicationProtectedDataDidBecomeAvailable:(UIApplication *)application
{
}

// Required according to docs...
- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    [UIDevice.currentDevice beginGeneratingDeviceOrientationNotifications];
    UIApplication.sharedApplication.idleTimerDisabled = YES;
    UIApplication.sharedApplication.statusBarOrientation = UIInterfaceOrientationLandscapeRight;
    
    windowInstance();
    
    [[NSTimer scheduledTimerWithTimeInterval: windowInstance().updateInterval() / 1000.0
              target: self
              selector: @selector(doTick:)
              userInfo: nil
              repeats: YES] retain];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	if (Gosu::Song::currentSong())
    {
        Gosu::Song::currentSong()->pause();
        pausedSong = true;
    }
    paused = true;
    windowInstance().loseFocus();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	if (pausedSong)
    {
        if (Gosu::Song::currentSong())
            Gosu::Song::currentSong()->play();
        pausedSong = false;
    }
    paused = false;
}

- (void)doTick:(NSTimer*)timer {
    if (!paused)
        windowInstance().update();
    [gosuView drawView];
    Gosu::Song::update();
    windowInstance().input().update();
}
@end

Gosu::Window::Window(unsigned width, unsigned height,
    bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
	pimpl->window.reset([[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]);
    pimpl->controller.reset([[GosuViewController alloc] init]);
    gosuView = (GosuView*)pimpl->controller.obj().view;
	[pimpl->window.obj() addSubview: gosuView];
    
    pimpl->graphics.reset(new Graphics(screenWidth(), screenHeight(), false));
    pimpl->graphics->setResolution(screenHeight(), screenWidth());
    pimpl->audio.reset(new Audio());
    pimpl->input.reset(new Input(gosuView, updateInterval));
    pimpl->input->onTouchBegan = std::tr1::bind(&Window::touchBegan, this, _1);
    pimpl->input->onTouchMoved = std::tr1::bind(&Window::touchMoved, this, _1);
    pimpl->input->onTouchEnded = std::tr1::bind(&Window::touchEnded, this, _1);
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
}

void Gosu::Window::close()
{
    throw std::logic_error("Cannot close windows manually on iOS");
}
