#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Timing.hpp>
#include <windows.h>

unsigned long Gosu::milliseconds()
{
    static unsigned long start = [] {
        timeBeginPeriod(1);
        return timeGetTime();
    }();
    return timeGetTime() - start;
}

#endif
