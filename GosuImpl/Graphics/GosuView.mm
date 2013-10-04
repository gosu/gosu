#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <UIKit/UIKit.h>

#import <Gosu/Graphics.hpp>
#import <Gosu/Window.hpp>
#import "Common.hpp"
#import "GosuView.hpp"

Gosu::Window& windowInstance();

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
}

int Gosu::clipRectBaseFactor()
{
    static int result = 0;
    if (result == 0)
    {
        if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
            result = [UIScreen mainScreen].scale;
        else
            result = 1;
    }
    return result;
}

// A controller to allow for autorotation.
@implementation GosuViewController
- (BOOL)prefersStatusBarHidden
{
    return YES;
}

- (void)loadView
{
    self.view = [[GosuView alloc] init];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return UIInterfaceOrientationIsLandscape(interfaceOrientation);
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

- (BOOL)shouldAutorotate
{
    return YES;
}

- (void)didReceiveMemoryWarning
{
    windowInstance().releaseMemory();
}
@end

// A class extension to declare private methods
@interface GosuView ()
@property (nonatomic, strong) EAGLContext *context;
@end

@implementation GosuView
{
    // The pixel dimensions of the backbuffer
    GLint backingWidth;
    GLint backingHeight;

    // OpenGL names for the renderbuffer and framebuffers used to render to this view
    GLuint viewRenderbuffer, viewFramebuffer;

    NSMutableSet* currentTouches;
    Gosu::Touches* currentTouchesVector;
}
    
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)init
{
    if ((self = [super initWithFrame: [[UIScreen mainScreen] bounds]])) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        /*eaglLayer.drawableProperties = dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];*/
        
        self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!self.context || ![EAGLContext setCurrentContext:self.context]) {
            [self release];
            return nil;
        }
    }
    return self;
}

- (void)drawView
{
    if (not windowInstance().needsRedraw())
        return;
    
    Gosu::FPS::registerFrame();
    
    [EAGLContext setCurrentContext:self.context];
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glViewport(0, 0, backingWidth, backingHeight);
    
    if (windowInstance().graphics().begin()) {
        windowInstance().draw();
        windowInstance().graphics().end();
    }
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [self.context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)layoutSubviews
{
    [EAGLContext setCurrentContext:self.context];
    [self destroyFramebuffer];
    if ([self respondsToSelector:@selector(contentScaleFactor)])
        self.contentScaleFactor = Gosu::clipRectBaseFactor();
    [self createFramebuffer];
    [self drawView];
}

- (BOOL)createFramebuffer
{
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [self.context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}

- (void)destroyFramebuffer
{
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
}

- (void)dealloc
{
    delete currentTouchesVector;
    [currentTouches release];
    [EAGLContext setCurrentContext:nil];
    [self.context release];
    [super dealloc];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    windowInstance().input().feedTouchEvent(0, touches);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    windowInstance().input().feedTouchEvent(1, touches);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    windowInstance().input().feedTouchEvent(2, touches);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    // TODO: Should be differentiated on the Gosu side.
    [self touchesEnded:touches withEvent:event];
}

- (BOOL)isMultipleTouchEnabled
{
    return YES;
}

- (BOOL)isExclusiveTouch
{
    return YES;
}

@end
