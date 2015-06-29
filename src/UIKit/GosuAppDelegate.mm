#import "GosuAppDelegate.h"
#import <Gosu/Window.hpp>


@implementation GosuAppDelegate

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
    self.window = (UIWindow *)windowInstance().UIWindow();
    [self.window makeKeyAndVisible];
}

@end


int main(int argc, char *argv[])
{
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, @"GosuAppDelegate");
    }
}
