#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <Gosu/Window.hpp>

// UIView subclass that contains a CAEAGLLayer.

@interface GosuView : UIView {
    
@private
    /* The pixel dimensions of the backbuffer */
    GLint backingWidth;
    GLint backingHeight;
    
    EAGLContext *context;
    
    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;
    
    NSMutableSet* currentTouches;
    Gosu::Touches* currentTouchesVector;
}

- (const Gosu::Touches&)currentTouches;

- (void)drawView;

// This method is necessary because if pressing a LOT of touches, some
// of them may end up not being sent to touchesEnded(), which is confusing
// to application programmers.
- (void)removeDeadTouches;

@end
