#import "GosuGLView.h"

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>


static EAGLContext __weak *globalContext;


namespace Gosu
{
    void ensure_current_context()
    {
        [EAGLContext setCurrentContext:globalContext];
    }
    
    int clip_rect_base_factor()
    {
        static int result = [UIScreen mainScreen].scale;
        return result;
    }
}


@implementation GosuGLView
{
    EAGLContext *_context;
    
    GLint _backingWidth;
    GLint _backingHeight;

    GLuint _viewRenderbuffer;
    GLuint _viewFramebuffer;
}
    
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame])) {
        [self initializeGosuGLView];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if ((self = [super initWithCoder:aDecoder])) {
        [self initializeGosuGLView];
    }
    
    return self;
}

- (void)initializeGosuGLView
{
    CAEAGLLayer *layer = (CAEAGLLayer *)self.layer;
    layer.opaque = YES;
    
    globalContext = _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    [EAGLContext setCurrentContext:_context];
    
    self.exclusiveTouch = YES;
    self.multipleTouchEnabled = YES;
}

- (void)dealloc
{
    [EAGLContext setCurrentContext:nil];
}

- (void)redrawGL:(void (^)())code
{
    [EAGLContext setCurrentContext:_context];
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
    
    glViewport(0, 0, _backingWidth, _backingHeight);
    
    if (code) {
        code();
    }
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer);
    
    [_context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)layoutSubviews
{
    [EAGLContext setCurrentContext:_context];
    [self destroyFramebuffer];
    [self createFramebuffer];
    self.contentScaleFactor = Gosu::clip_rect_base_factor();
}

- (BOOL)createFramebuffer
{
    glGenFramebuffersOES(1, &_viewFramebuffer);
    glGenRenderbuffersOES(1, &_viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _viewRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_backingHeight);
    
    return (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES);
}

- (void)destroyFramebuffer
{
    glDeleteFramebuffersOES(1, &_viewFramebuffer);
    _viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &_viewRenderbuffer);
    _viewRenderbuffer = 0;
}

@end
