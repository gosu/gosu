#include <Gosu/Inspection.hpp>
#include <Gosu/Timing.hpp>

namespace Gosu
{
    namespace FPS
    {
        int fps, accum, sec;

        void registerFrame()
        {
            ++accum;
            int newSec = Gosu::milliseconds() / 1000;
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
