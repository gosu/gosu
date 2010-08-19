#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <UIKit/UIKit.h>

#import <Gosu/Graphics.hpp>
#import <GosuImpl/Graphics/Common.hpp>
#import <GosuImpl/Graphics/GosuView.hpp>

Gosu::Window& windowInstance();

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
- (void)loadView {
    self.view = [[GosuView alloc] init];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return NO;//UIInterfaceOrientationIsLandscape(interfaceOrientation);
}

- (void)didReceiveMemoryWarning {
    windowInstance().releaseMemory();
}
@end

// A class extension to declare private methods
@interface GosuView ()

@property (nonatomic, retain) EAGLContext *context;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation GosuView

@synthesize context;

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (id)init {
    if ((self = [super initWithFrame: [[UIScreen mainScreen] bounds]])) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        /*eaglLayer.drawableProperties = [NSDictionary new]; /*dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];*/
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
    }
    return self;
}

- (void)drawView {
    if (not windowInstance().needsRedraw())
        return;
    
    [EAGLContext setCurrentContext:context];
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glViewport(0, 0, backingWidth, backingHeight);
    
    if (windowInstance().graphics().begin()) {
        windowInstance().draw();
        windowInstance().graphics().end();
    }
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)layoutSubviews {
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    if ([self respondsToSelector:@selector(contentScaleFactor)])
        self.contentScaleFactor = Gosu::clipRectBaseFactor();
    [self createFramebuffer];
    [self drawView];
}

- (BOOL)createFramebuffer {
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}

- (void)destroyFramebuffer {
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
}

- (void)dealloc {
    delete currentTouchesVector;
    [currentTouches release];
    [EAGLContext setCurrentContext:nil];
    [context release];
    [super dealloc];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {    
    windowInstance().input().feedTouchEvent(0, touches);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    windowInstance().input().feedTouchEvent(1, touches);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    windowInstance().input().feedTouchEvent(2, touches);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    // TODO: Should be differentiated on the Gosu side.
    [self touchesEnded: touches withEvent: event];
}

- (BOOL)isMultipleTouchEnabled {
    return YES;
}

- (BOOL)isExclusiveTouch {
    return YES;
}

@end
