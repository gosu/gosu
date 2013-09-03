#include "Orientation.hpp"
#include <Gosu/Timing.hpp>
#import <UIKit/UIKit.h>

namespace
{
    Gosu::Orientation orientationToGosu(UIDeviceOrientation orientation)
    {
        return orientation == UIDeviceOrientationLandscapeLeft ? Gosu::orLandscapeLeft : Gosu::orLandscapeRight;
    }
}

Gosu::Orientation Gosu::currentOrientation()
{
    return orientationToGosu([[UIDevice currentDevice] orientation]);
}
