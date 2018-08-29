#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Timing.hpp>
#include <cstdlib>
#include <windows.h>

void Gosu::sleep(unsigned milliseconds)
{
    Sleep(milliseconds);
}

unsigned long Gosu::milliseconds()
{
    static unsigned long start = [] {
        timeBeginPeriod(1);
        return timeGetTime();
    }();
    return timeGetTime() - start;
}

#endif
