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
    static bool init = false;
    if (!init)
    {
        if (::timeBeginPeriod(1) != TIMERR_NOERROR)
            std::atexit(resetTGT);
        init = true;
    }
    return ::timeGetTime();
}
