#include <UIKit/UIKit.h>

typedef UIAccelerationValue Acceleration[3];

@interface AccelerometerReader : NSObject {
    Acceleration acceleration;
}
-(AccelerometerReader*)initWithUpdateInterval:(float)updateInterval;
-(const Acceleration&)acceleration;
@end
