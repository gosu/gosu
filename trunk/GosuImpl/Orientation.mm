#include <GosuImpl/Orientation.hpp>
#include <Gosu/Timing.hpp>
#import <UIKit/UIKit.h>

namespace
{
    Gosu::Orientation orientationToGosu(UIDeviceOrientation orientation)
    {
        return orientation == UIDeviceOrientationLandscapeLeft ? Gosu::orLandscapeLeft : Gosu::orLandscapeRight;
    }

    const unsigned CHANGE_AFTER_MS = 500;
    Gosu::Orientation orientation = orientationToGosu([[UIDevice currentDevice] orientation]);
    unsigned changedAtTime = Gosu::milliseconds();
}

Gosu::Orientation Gosu::currentOrientation()
{
    UIDeviceOrientation newOrientation = [[UIDevice currentDevice] orientation];
    unsigned now = Gosu::milliseconds();
    
    if (now - changedAtTime >= CHANGE_AFTER_MS)
    {
        orientation = orientationToGosu(newOrientation);
        changedAtTime = now;
    }
    if (!UIDeviceOrientationIsLandscape(newOrientation) || orientationToGosu(newOrientation) == orientation)
        changedAtTime = now;
    
    return orientation;
}
