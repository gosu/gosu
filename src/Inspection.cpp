#include <Gosu/Inspection.hpp>
#include <Gosu/Timing.hpp>

namespace Gosu
{
    namespace FPS
    {
        int fps, accum;
        unsigned long sec;

        void registerFrame()
        {
            ++accum;
            unsigned long newSec = Gosu::milliseconds() / 1000;
            if (sec != newSec)
            {
                sec = newSec;
                fps = accum;
                accum = 0;
            }
        }
    }
    
    int fps()
    {
        return FPS::fps;
    }
}
