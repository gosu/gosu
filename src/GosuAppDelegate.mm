#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#import <UIKit/UIKit.h>
#import <Gosu/Gosu.hpp>

// You will have to implement this method in C++.
Gosu::Window& window_instance();


@interface GosuAppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow *window;
@end


@implementation GosuAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    self.window = (__bridge UIWindow*)window_instance().uikit_window();
    [self.window makeKeyAndVisible];
    
    return YES;
}

@end

int main(int argc, char* argv[])
{
    @autoreleasepool {
        Gosu::use_resource_directory();
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([GosuAppDelegate class]));
    }
}

#endif
