#include <Gosu/Timing.hpp>
#include <windows.h>
#include <cstdlib>

void Gosu::sleep(unsigned milliseconds)
{
    ::Sleep(milliseconds);
}

namespace
{
    void resetTGT()
    {
        ::timeEndPeriod(1);
    }
}

unsigned long Gosu::milliseconds()
{
		static unsigned long start = 0;

    if (!start)
    {
        if (::timeBeginPeriod(1) != TIMERR_NOERROR)
            std::atexit(resetTGT);
				start = ::timeGetTime();
    }
    // Truncate to 2^30, C++ users shouldn't mind and Ruby users will
    // have a happy GC on 32-bit systems.
    // No, don't ask why this is an unsigned long then :)
    return (::timeGetTime() - start) & 0x1fffffff;
}
