#include <Gosu/Timing.hpp>
#include <unistd.h>
#include <sys/time.h>

void Gosu::sleep(unsigned milliseconds)
{
	usleep(milliseconds * 1000);
}

unsigned long Gosu::milliseconds()
{
	timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_usec / 1000 + tp.tv_sec * 1000;
}
