#import <UIKit/UIKit.h>
#import <Gosu/Fwd.hpp>


@interface GosuViewController : UIViewController

- (void)trackTextInput:(Gosu::Input&)input;

@property (nonatomic, assign) void *gosuWindow;

@end
