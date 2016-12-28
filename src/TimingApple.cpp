#include <Gosu/Timing.hpp>
#include <unistd.h>

void Gosu::sleep(unsigned milliseconds)
{
	usleep(milliseconds * 1000);
}

// Thanks to this blog for the unconvoluted code example:
// http://shiftedbits.org/2008/10/01/mach_absolute_time-on-the-iphone/

#include <mach/mach_time.h>

unsigned long Gosu::milliseconds()
{
    static uint64_t first_tick = 0;
    static mach_timebase_info_data_t info;
    
    if (first_tick == 0)
    {
        mach_timebase_info(&info);
        first_tick = mach_absolute_time();
    }

    uint64_t runtime = mach_absolute_time() - first_tick;
	return runtime * info.numer / info.denom / 1000000.0;
}
