#ifndef GOSUIMPL_WINDOWFRAMEMANAGER_HPP
#define GOSUIMPL_WINDOWFRAMEMANAGER_HPP

#include <Gosu/Timing.hpp>

namespace Gosu
{
    class FrameManager
    {
        double interval;
    
        unsigned beginOfLastTick;
        double accumulatedDebt;
        unsigned skippedFrames;
        
        unsigned beginOfTick;

        enum
        {
            SKIPPING_CORRECTION = 10,
            MAX_SKIP_FRAMES = 3
        };
        
    public:
        FrameManager()
        : accumulatedDebt(0), skippedFrames(0)
        {
        }
        
        void start(double interval)
        {
            /*this->interval = interval;
            beginOfLastTick = Gosu::milliseconds();*/
        }
        
        bool shouldDrawFrameStartingNow()
        {
            return true;
            /*
            beginOfTick = Gosu::milliseconds();
            if (beginOfTick < beginOfLastTick)
            {
                // Timer ran over, whoops
                beginOfLastTick = beginOfTick;
                return false;
            }
            
            accumulatedDebt +=
                beginOfTick - beginOfLastTick - interval;
            // Sometimes very long ticks will throw us off our course.
            if (accumulatedDebt >= interval * MAX_SKIP_FRAMES)
                accumulatedDebt = interval * MAX_SKIP_FRAMES;
            // Automatically work off some debt per frame so minimal delays don't cause an extra frame
            // eventually.
            else if (accumulatedDebt > SKIPPING_CORRECTION)
                accumulatedDebt -= SKIPPING_CORRECTION;
            // Also, prevent the correction from growing into the negative and eat up later attempts to
            // skip things.
            else if (accumulatedDebt < -SKIPPING_CORRECTION)
                accumulatedDebt += SKIPPING_CORRECTION;
        
            if (accumulatedDebt >= interval / 2 and
                skippedFrames < MAX_SKIP_FRAMES)
            {
                skippedFrames++;
                return false;
            }
            else
            {
                skippedFrames = 0;
                return true;
            }
            */
        }
        
        void frameEnded()
        {
            /*
            beginOfLastTick = beginOfTick;
            */
        }
    };
}

#endif
