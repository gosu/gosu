#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include <Gosu/Timing.hpp>
#include <sys/time.h>
#include <unistd.h>

void Gosu::sleep(unsigned milliseconds)
{
    usleep(milliseconds * 1000);
}

unsigned long Gosu::milliseconds()
{
    timeval tp;
    gettimeofday(&tp, nullptr);
    unsigned long ms = tp.tv_usec / 1000UL + tp.tv_sec * 1000UL;

    static unsigned long start = ms;
    return ms - start;
}

#endif
