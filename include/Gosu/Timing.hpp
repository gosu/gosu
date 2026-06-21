#pragma once

namespace Gosu
{
    /// Freezes the current thread for the given amount of milliseconds.
    void sleep(unsigned milliseconds);

    /// Returns the milliseconds since first calling this function.
    /// Can wrap after running for a long time.
    unsigned long milliseconds();
}
