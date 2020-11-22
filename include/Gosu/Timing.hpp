//! \file Timing.hpp
//! Functions for timing.

#pragma once

namespace Gosu
{
    //! Freezes the current thread for at least the specified time.
    void sleep(unsigned milliseconds);

    //! Returns the milliseconds since first calling this function.
    //! Can wrap after running for a long time.
    unsigned long milliseconds();
}
