#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Timing.hpp>
#include <windows.h>
#include <cstdlib>

void Gosu::sleep(unsigned milliseconds)
{
    Sleep(milliseconds);
}

unsigned long Gosu::milliseconds()
{
    static unsigned long start = (timeBeginPeriod(1), timeGetTime());
    return timeGetTime() - start;
}

#endif
