#import <UIKit/UIKit.h>


@class GosuGLView;


@interface GosuViewController : UIViewController

- (GosuGLView *)GLView;

@property (nonatomic, assign) void *gosuWindow;

@end
