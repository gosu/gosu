#include <Gosu/Timing.hpp>
#include "GraphicsImpl.hpp"

namespace Gosu
{
    namespace
    {
        int current_fps = 0;
    }

    void register_frame()
    {
        static unsigned long current_second = Gosu::milliseconds() / 1000;
        static int frames_in_current_second = 0;

        unsigned long new_sec = Gosu::milliseconds() / 1000;
        if (current_second != new_sec) {
            current_second = new_sec;
            current_fps = frames_in_current_second;
            frames_in_current_second = 0;
        }
        else {
            ++frames_in_current_second;
        }
    }

    int fps()
    {
        return current_fps;
    }
}
