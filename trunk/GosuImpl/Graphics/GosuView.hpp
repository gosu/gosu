#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <Gosu/Window.hpp>

// UIViewController subclass that creates a GosuView and helps it manage rotation.

@interface GosuViewController : UIViewController
@end

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

- (void)drawView;
@end
