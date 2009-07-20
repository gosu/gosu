#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <UIKit/UIKit.h>

#import <Gosu/Graphics.hpp>
#import <GosuImpl/Graphics/GosuView.hpp>

Gosu::Window& windowInstance();

namespace {
    Gosu::Touches translateTouches(NSSet* touches, UIView* view)
    {
        Gosu::Touches result;
        for (UITouch* uiTouch in touches)
        {                
            CGPoint point = [uiTouch locationInView: view];
            Gosu::Touch touch = { uiTouch, point.y, [view bounds].size.width - point.x };
            result.push_back(touch);
        }
        return result;
    }
}

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

- (id)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
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

- (const Gosu::Touches&)currentTouches {
    if (!currentTouchesVector)
        currentTouchesVector = new Gosu::Touches(translateTouches(currentTouches, self));
    return *currentTouchesVector;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    delete currentTouchesVector; currentTouchesVector = 0;
    if (!currentTouches) currentTouches = [[NSMutableSet alloc] init];

    [currentTouches unionSet: touches];

    windowInstance().touchesBegan(translateTouches(touches, self));
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    delete currentTouchesVector; currentTouchesVector = 0;

    windowInstance().touchesMoved(translateTouches(touches, self));
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    delete currentTouchesVector; currentTouchesVector = 0;
    if (!currentTouches) currentTouches = [[NSMutableSet alloc] init];

    [currentTouches minusSet: touches];

    windowInstance().touchesEnded(translateTouches(touches, self));
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    // TODO: Should be differentiated on the Gosu side.
    [self touchesEnded: touches withEvent: event];
}

- (void)removeDeadTouches {
    NSMutableSet* deadTouches = 0;

    for (UITouch* touch in currentTouches)
    {
        UITouchPhase phase = [touch phase];
        if (phase == UITouchPhaseBegan ||
            phase == UITouchPhaseMoved ||
            phase == UITouchPhaseStationary)
            continue;
            
        // Something was deleted, we will need the set.
        if (deadTouches == 0)
            deadTouches = [[NSMutableSet alloc] init];
        [deadTouches addObject:touch];
    }
    
    // Has something been deleted?
    if (deadTouches)
    {
        delete currentTouchesVector;
        [currentTouches minusSet: deadTouches];
        windowInstance().touchesEnded(translateTouches(deadTouches, self));
    }
}

- (bool)isMultipleTouchEnabled {
    return YES;
}

- (bool)isExclusiveTouch {
    return YES;
}

@end
