#include <Gosu/Inspection.hpp>
#include <Gosu/Timing.hpp>

namespace Gosu
{
    namespace FPS
    {
        int fps, accum;
        unsigned long sec;

        void register_frame()
        {
            ++accum;
            unsigned long new_sec = Gosu::milliseconds() / 1000;
            if (sec != new_sec) {
                sec = new_sec;
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
