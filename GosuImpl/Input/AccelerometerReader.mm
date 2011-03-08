// Thanks for inspiration to Phil Cooper-King.
// This is adapted from his MIT-licensed Gosu fork
// on GitHub.

#import <GosuImpl/Input/AccelerometerReader.hpp>

@implementation AccelerometerReader
- (AccelerometerReader*)initWithUpdateInterval: (float)updateInterval
{
    if (self = [super init])
    {
        [[UIAccelerometer sharedAccelerometer] setUpdateInterval: updateInterval];
        [[UIAccelerometer sharedAccelerometer] setDelegate:self];
        acceleration[0] = acceleration[1] = acceleration[2] = 0.0f;
    }
    return self;
}

- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)uiAccel
{
    float factor = 0.3f;
    acceleration[0] = (uiAccel.x * factor) + (acceleration[0] * (1 - factor));
    acceleration[1] = (uiAccel.y * factor) + (acceleration[1] * (1 - factor));
    acceleration[2] = (uiAccel.z * factor) + (acceleration[2] * (1 - factor));
}

- (const Acceleration&)acceleration
{
    return acceleration;
}
@end
