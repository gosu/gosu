#import <UIKit/UIKit.h>


@interface GosuGLView : UIView

- (void)redrawGL:(void (^)())code;

@end
