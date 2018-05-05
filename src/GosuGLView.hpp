#import <UIKit/UIKit.h>
#import <Gosu/Fwd.hpp>

@interface GosuGLView : UIView <UITextInput>

- (instancetype)initWithFrame:(CGRect)frame input:(Gosu::Input&)input;
- (instancetype)initWithFrame:(CGRect)frame NS_UNAVAILABLE;
- (instancetype)initWithCoder:(NSCoder *)aDecoder NS_UNAVAILABLE;

- (void)redrawGL:(void (^)())code;

@end
