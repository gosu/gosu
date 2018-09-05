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
    static unsigned long start = 0;

    timeval tp;
    gettimeofday(&tp, nullptr);

    if (start == 0) {
        start = tp.tv_usec / 1000UL + tp.tv_sec * 1000UL;
    }

    return tp.tv_usec / 1000UL + tp.tv_sec * 1000UL - start;
}

#endif
