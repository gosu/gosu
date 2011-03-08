#include <GosuImpl/Orientation.hpp>
#include <Gosu/Timing.hpp>
#import <UIKit/UIKit.h>

namespace
{
    static const unsigned CHANGE_AFTER_MS = 500;
    
    Gosu::Orientation orientationToGosu(UIDeviceOrientation orientation)
    {
        return orientation == UIDeviceOrientationLandscapeLeft ? Gosu::orLandscapeLeft : Gosu::orLandscapeRight;
    }
}

Gosu::Orientation Gosu::currentOrientation()
{
    UIDeviceOrientation newOrientation = [[UIDevice currentDevice] orientation];
    unsigned now = Gosu::milliseconds();
    
    static Orientation orientation = Gosu::Orientation(-1);
    static unsigned waitingForChangeSince = now;

    if (!UIDeviceOrientationIsLandscape(newOrientation) || orientationToGosu(newOrientation) == orientation)
        waitingForChangeSince = now;
    
    if (now - waitingForChangeSince >= CHANGE_AFTER_MS or orientation == Gosu::Orientation(-1))
    {
        orientation = orientationToGosu(newOrientation);
        [UIApplication sharedApplication].statusBarOrientation =
            (newOrientation == UIDeviceOrientationLandscapeLeft ? UIInterfaceOrientationLandscapeRight : UIInterfaceOrientationLandscapeLeft);
    }
    
    return orientation;
}
