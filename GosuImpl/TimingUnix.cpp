#include <Gosu/Timing.hpp>
#include <unistd.h>
#include <sys/time.h>

void Gosu::sleep(unsigned milliseconds)
{
	usleep(milliseconds * 1000);
}

unsigned long Gosu::milliseconds()
{
		static unsigned long start = 0;

    timeval tp;
    gettimeofday(&tp, NULL);

		if (!start)
			start = tp.tv_usec / 1000UL + tp.tv_sec * 1000UL;
		
    // Truncate to 2^30, C++ users shouldn't mind and Ruby users will
    // have a happy GC on 32-bit systems.
    // No, don't ask why this is an unsigned long then :)
    return (tp.tv_usec / 1000UL + tp.tv_sec * 1000UL - start) & 0x1fffffff;
}
