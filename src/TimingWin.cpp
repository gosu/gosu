#include <Gosu/Timing.hpp>
#include <Gosu/platform.hpp>
#include <windows.h>
#include <cstdlib>

void Gosu::sleep(unsigned milliseconds)
{
    Sleep(milliseconds);
}

unsigned long Gosu::milliseconds()
{
#if !defined(GOSU_IS_UWP)
    static unsigned long start = (timeBeginPeriod(1), timeGetTime());
    return timeGetTime() - start;
#else
	// not really sure about this
	static unsigned long start = GetTickCount64();
	return GetTickCount64() - start;
#endif
}
