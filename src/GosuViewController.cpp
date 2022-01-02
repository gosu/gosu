#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#import "GosuGLView.hpp"
#import "GosuViewController.hpp"
#import "GraphicsImpl.hpp"
#import "AudioImpl.hpp"
#import <Gosu/Gosu.hpp>

#import <AudioToolbox/AudioSession.h>
#import <OpenAL/alc.h>

namespace Gosu
{
    namespace FPS
    {
        void register_frame();
    }
}

static void handle_audio_interruption(void* unused, UInt32 inInterruptionState)
{
    if (inInterruptionState == kAudioSessionBeginInterruption) {
        alcMakeContextCurrent(nullptr);
    }
    else if (inInterruptionState == kAudioSessionEndInterruption) {
        alcMakeContextCurrent(Gosu::al_context());
    }
}

@implementation GosuViewController
{
    BOOL _paused;
    BOOL _musicPaused;
    id _timerOrDisplayLink;
}

#pragma mark - UIViewController

- (void)loadView
{
    self.view = [[GosuGLView alloc] initWithFrame:[UIScreen mainScreen].bounds];
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}

- (BOOL)shouldAutorotate
{
    return YES;
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

- (Gosu::Window&)gosuWindowReference
{
    NSAssert(self.gosuWindow, @"gosuWindow needs to be set before showing GosuViewController");
    
    return *(Gosu::Window*)self.gosuWindow;
}

#pragma mark - Notifications (handle pausing)

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // TODO - replace with AVAudioSession https://stackoverflow.com/q/19710046
    AudioSessionInitialize(nullptr, nullptr, handle_audio_interruption, nullptr);
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive:)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive:)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground:)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground:)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    if (_musicPaused) {
        if (Gosu::Song::current_song()) {
            Gosu::Song::current_song()->play();
        }
        _musicPaused = NO;
    }
    _paused = NO;
}

- (void)applicationWillResignActive:(NSNotification*)notification
{
    if (Gosu::Song::current_song()) {
        Gosu::Song::current_song()->pause();
        _musicPaused = YES;
    }
    _paused = YES;
    
    self.gosuWindowReference.lose_focus();
}

- (void)applicationWillEnterForeground:(NSNotification*)notification
{
    [self setupTimerOrDisplayLink];
    
    self.gosuWindowReference.gain_focus();
}

- (void)applicationDidEnterBackground:(NSNotification*)notification
{
    [_timerOrDisplayLink invalidate];
    _timerOrDisplayLink = nil;
}

#pragma mark - Redraw & update "loop"

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self setupTimerOrDisplayLink];
    
    [UIApplication sharedApplication].statusBarHidden = YES;
}

- (void)setupTimerOrDisplayLink
{
    if (_timerOrDisplayLink) return;
    
    NSInteger targetFPS = round(1000.0 / self.gosuWindowReference.update_interval());
    
    if (60 % targetFPS != 0) {
        NSTimeInterval interval = self.gosuWindowReference.update_interval() / 1000.0;
        NSTimer* timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                                          target:self
                                                        selector:@selector(updateAndDraw:)
                                                        userInfo:nil
                                                         repeats:YES];
        
        _timerOrDisplayLink = timer;
    }
    else {
        CADisplayLink* displayLink =
            [CADisplayLink displayLinkWithTarget:self selector:@selector(updateAndDraw:)];
        displayLink.frameInterval = 60 / targetFPS;
        [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
        
        _timerOrDisplayLink = displayLink;
    }
}

- (void)updateAndDraw:(id)sender
{
    Gosu::Window& window = self.gosuWindowReference;
    
    window.input().update();
    
    if (!_paused) {
        window.update();
    }
    
    if (window.needs_redraw()) {
        [(GosuGLView*)self.view redrawGL:^{
            window.graphics().frame([&window] {
                window.draw();
                Gosu::FPS::register_frame();
            });
        }];
    }
    
    Gosu::Song::update();
}

#pragma mark - Input setup

- (void)trackTextInput:(Gosu::Input&)input
{
    ((GosuGLView*)self.view).input = &input;
}

#pragma mark - Touch forwarding

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    auto& input = self.gosuWindowReference.input();
    input.feed_touch_event(input.on_touch_began, (__bridge void*)touches);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    auto& input = self.gosuWindowReference.input();
    input.feed_touch_event(input.on_touch_moved, (__bridge void*)touches);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    auto& input = self.gosuWindowReference.input();
    input.feed_touch_event(input.on_touch_ended, (__bridge void*)touches);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    auto& input = self.gosuWindowReference.input();
    input.feed_touch_event(input.on_touch_cancelled, (__bridge void*)touches);
}

@end

#endif
