#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#import "GosuGLView.hpp"
#import <Gosu/Input.hpp>
#import <Gosu/TextInput.hpp>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>

#import <algorithm>

using namespace std;


static EAGLContext __weak* globalContext;

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

#pragma mark - GosuTextPosition

@interface GosuTextPosition : UITextPosition

@property (nonatomic) NSInteger position;

@end

@implementation GosuTextPosition

- (instancetype)initWithPosition:(NSInteger)position
{
    if (self = [super init]) {
        _position = position;
    }
    return self;
}

@end

#pragma mark - GosuTextRange

@interface GosuTextRange : UITextRange

@property (nonatomic, readonly) GosuTextPosition* start;
@property (nonatomic, readonly) GosuTextPosition* end;

@end

@implementation GosuTextRange

@synthesize start = _start;
@synthesize end   = _end;

- (instancetype)initWithStart:(NSInteger)start end:(NSInteger)end
{
    if (self = [super init]) {
        _start = [[GosuTextPosition alloc] initWithPosition:start];
        _end   = [[GosuTextPosition alloc] initWithPosition:end];
    }
    return self;
}

- (BOOL)isEmpty
{
    return _start.position == _end.position;
}

@end


@implementation GosuGLView
{
    Gosu::Input* _input;
    EAGLContext* _context;

    GLint _backingWidth;
    GLint _backingHeight;

    GLuint _viewRenderbuffer;
    GLuint _viewFramebuffer;
}

- (instancetype)initWithFrame:(CGRect)frame input:(Gosu::Input&)input
{
    if (self = [super initWithFrame:frame]) {
        CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
        layer.opaque = YES;
        
        _input = &input;
        
        globalContext = _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        [EAGLContext setCurrentContext:_context];
        
        self.exclusiveTouch = YES;
        self.multipleTouchEnabled = YES;
    }
    
    return self;
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

#pragma mark - UIView

+ (Class)layerClass
{
    return [CAEAGLLayer class];
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
    [_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES,
                                 _viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES,
                                    &_backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES,
                                    &_backingHeight);
    
    return (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES);
}

- (void)destroyFramebuffer
{
    glDeleteFramebuffersOES(1, &_viewFramebuffer);
    _viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &_viewRenderbuffer);
    _viewRenderbuffer = 0;
}

#pragma mark - NSResponder / UITextInput

- (BOOL)canBecomeFirstResponder
{
    return _input->text_input() != nullptr;
}

- (NSString*)textInRange:(UITextRange*)range
{
    if (_input->text_input() == nullptr) return nil;
    
    NSString* string = @(_input->text_input()->text().c_str());
    auto start = ((GosuTextPosition*)range.start).position;
    auto length = ((GosuTextPosition*)range.end).position - start;
    return [string substringWithRange:NSMakeRange(start, length)];
}

- (void)replaceRange:(UITextRange*)range withText:(NSString*)text
{
    NSString* string = @(_input->text_input()->text().c_str());
    auto start = ((GosuTextPosition*)range.start).position;
    auto length = ((GosuTextPosition*)range.end).position - start;
    string = [string stringByReplacingCharactersInRange:NSMakeRange(start, length) withString:text];
    _input->text_input()->set_text(string.UTF8String ?: "");
    _input->text_input()->set_selection_start(static_cast<unsigned>(start + length));
}

- (UITextRange*)selectedTextRange
{
    if (_input->text_input() == nullptr) return nil;
    
    auto& text_input = *_input->text_input();
    auto start = min(text_input.selection_start(), text_input.caret_pos());
    auto end   = max(text_input.selection_start(), text_input.caret_pos());
    return [[GosuTextRange alloc] initWithStart:start end:end];
}

- (void)setSelectedTextRange:(UITextRange*)selectedTextRange
{
    if (_input->text_input() == nullptr) return;

    GosuTextRange* range = (GosuTextRange*)selectedTextRange;
    _input->text_input()->set_selection_start(static_cast<unsigned>(range.start.position));
    _input->text_input()->set_caret_pos(static_cast<unsigned>(range.end.position));
}



@end

#endif
