#import "GosuViewController.h"
#import "GosuGLView.h"
#import "../Graphics/Common.hpp"
#import <Gosu/Gosu.hpp>

#import <OpenAL/alc.h>
#import <AudioToolbox/AudioSession.h>


namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
    
    ALCcontext *sharedContext();
}


// TODO: This has been written on iOS 3.x.
// Is this still the best way to handle interruptions?
static void handleAudioInterruption(void *unused, UInt32 inInterruptionState)
{
    if (inInterruptionState == kAudioSessionBeginInterruption) {
        alcMakeContextCurrent(NULL);
    }
    else if (inInterruptionState == kAudioSessionEndInterruption) {
        alcMakeContextCurrent(Gosu::sharedContext());
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
    self.view = [[[GosuGLView alloc] initWithFrame:[UIScreen mainScreen].bounds] autorelease];
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
    
    AudioSessionInitialize(NULL, NULL, handleAudioInterruption, NULL);
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground:) name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground:) name:UIApplicationDidEnterBackgroundNotification object:nil];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [super dealloc];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if (_musicPaused) {
        if (Gosu::Song::currentSong())
            Gosu::Song::currentSong()->play();
        _musicPaused = NO;
    }
    _paused = NO;
}

- (void)applicationWillResignActive:(NSNotification *)notification
{
    if (Gosu::Song::currentSong()) {
        Gosu::Song::currentSong()->pause();
        _musicPaused = YES;
    }
    _paused = YES;
    
    self.gosuWindowReference.loseFocus();
}

- (void)applicationWillEnterForeground:(NSNotification *)notification
{
    [self setupTimerOrDisplayLink];
}

- (void)applicationDidEnterBackground:(NSNotification *)notification
{
    [_timerOrDisplayLink invalidate];
    [_timerOrDisplayLink release];
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
    
    NSInteger targetFPS = round(1000.0 / self.gosuWindowReference.updateInterval());
    
    if (60 % targetFPS != 0) {
        NSTimeInterval interval = self.gosuWindowReference.updateInterval() / 1000.0;
        NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(updateAndDraw:) userInfo:nil repeats:YES];
        
        _timerOrDisplayLink = [timer retain];
    }
    else {
        CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(updateAndDraw:)];
        displayLink.frameInterval = 60 / targetFPS;
        [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
        
        _timerOrDisplayLink = [displayLink retain];
    }
}

- (void)updateAndDraw:(id)sender
{
    Gosu::Window& window = self.gosuWindowReference;
    
    window.input().update();
    
    if (!_paused) {
        window.update();
    }
    
    if (window.needsRedraw()) {
        [self.GLView redrawGL:^{
            if (window.graphics().begin()) {
                window.draw();
                window.graphics().end();
                
                Gosu::FPS::registerFrame();
            }
        }];
    }
    
    Gosu::Song::update();
}

#pragma mark - Touch forwarding

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feedTouchEvent(0, touches);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feedTouchEvent(1, touches);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.gosuWindowReference.input().feedTouchEvent(2, touches);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    // TODO: Should have a different event in Gosu! Not the same as Ended.
    [self touchesEnded:touches withEvent:event];
}

@end
