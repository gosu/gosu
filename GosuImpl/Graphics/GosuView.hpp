#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

// UIViewController subclass that creates a GosuView and helps it manage rotation.

@interface GosuViewController : UIViewController
@end

// UIView subclass that contains a CAEAGLLayer.

@interface GosuView : UIView
- (void)drawView;
@end
