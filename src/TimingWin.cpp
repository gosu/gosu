#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Timing.hpp>
#include <windows.h>

void Gosu::sleep(unsigned milliseconds)
{
    SleepEx(milliseconds, FALSE);
}

unsigned long Gosu::milliseconds()
{
    static const unsigned long start = [] {
        timeBeginPeriod(1);
        return timeGetTime();
    }();
    return timeGetTime() - start;
}

#endif
