#import "GosuViewController.h"
#import "GosuGLView.h"
#import "GraphicsImpl.hpp"
#import <Gosu/Gosu.hpp>

#import <OpenAL/alc.h>
#import <AudioToolbox/AudioSession.h>


namespace Gosu
{
    namespace FPS
    {
        void register_frame();
    }
    
    ALCcontext *shared_openal_context();
}


// TODO: This has been written on iOS 3.x.
// Is this still the best way to handle interruptions?
static void handle_audio_interruption(void *unused, UInt32 inInterruptionState)
{
    if (inInterruptionState == kAudioSessionBeginInterruption) {
        alcMakeContextCurrent(nullptr);
    }
    else if (inInterruptionState == kAudioSessionEndInterruption) {
        alcMakeContextCurrent(Gosu::shared_openal_context());
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

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return UIInterfaceOrientationIsLandscape(interfaceOrientation);
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

- (GosuGLView *)GLView
{
    return (GosuGLView *)self.view;
}

- (Gosu::Window &)gosuWindowReference
{
    NSAssert(self.gosuWindow,
             @"gosuWindow needs to be set before showing GosuViewController");
    
    return *(Gosu::Window *)self.gosuWindow;
}

#pragma mark - Notifications (handle pausing)

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    AudioSessionInitialize(NULL, NULL, handle_audio_interruption, NULL);
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground:) name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground:) name:UIApplicationDidEnterBackgroundNotification object:nil];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if (_musicPaused) {
        if (Gosu::Song::current_song())
            Gosu::Song::current_song()->play();
        _musicPaused = NO;
    }
    _paused = NO;
}

- (void)applicationWillResignActive:(NSNotification *)notification
{
    if (Gosu::Song::current_song()) {
        Gosu::Song::current_song()->pause();
        _musicPaused = YES;
    }
    _paused = YES;
    
    self.gosuWindowReference.lose_focus();
}

- (void)applicationWillEnterForeground:(NSNotification *)notification
{
    [self setupTimerOrDisplayLink];
}

- (void)applicationDidEnterBackground:(NSNotification *)notification
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
    if (_timerOrDisplayLink)
        return;
    
    NSInteger targetFPS = round(1000.0 / self.gosuWindowReference.update_interval());
    
    if (60 % targetFPS != 0) {
        NSTimeInterval interval = self.gosuWindowReference.update_interval() / 1000.0;
        NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(updateAndDraw:) userInfo:nil repeats:YES];
        
        _timerOrDisplayLink = timer;
    }
    else {
        CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(updateAndDraw:)];
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
        [self.GLView redrawGL:^{
            if (window.graphics().begin()) {
                window.draw();
                window.graphics().end();
                
                Gosu::FPS::register_frame();
            }
        }];
    }
    
    Gosu::Song::update();
}

#pragma mark - Touch forwarding

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feed_touch_event(0, (__bridge void *)touches);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feed_touch_event(1, (__bridge void *)touches);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feed_touch_event(2, (__bridge void *)touches);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feed_touch_event(3, (__bridge void *)touches);
}

@end
