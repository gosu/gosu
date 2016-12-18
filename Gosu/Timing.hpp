//! \file Timing.hpp
//! Functions for timing.

#pragma once

namespace Gosu
{
    //! Freezes the current thread for at least the specified time.
    void sleep(unsigned milliseconds);

    //! Incrementing, possibly wrapping millisecond timer.
    unsigned long milliseconds();
}
