#import "GosuAppDelegate.h"
#import <Gosu/Window.hpp>


@implementation GosuAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = (UIWindow *)windowInstance().UIWindow();
    [self.window makeKeyAndVisible];
    
    return YES;
}

@end


int main(int argc, char *argv[])
{
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([GosuAppDelegate class]));
    }
}
