#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#import "GosuAppDelegate.h"
#import <Gosu/Gosu.hpp>

Gosu::Window& window_instance();

@implementation GosuAppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    self.window = (__bridge UIWindow*) window_instance().UIWindow();
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
