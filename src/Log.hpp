#pragma once

#include <cstdio>
#include <cstdlib>
#include <utility>

namespace Gosu
{
    template<class... Args>
    void log(const char* format, Args&&... args)
    {
        // NOLINTNEXTLINE(concurrency-mt-unsafe) - we don't even follow the pointer
        static bool enable_log = (std::getenv("GOSU_DEBUG") != nullptr);
        if (enable_log) {
            std::fprintf(stderr, format, std::forward<Args>(args)...);
            std::fprintf(stderr, "\n");
        }
    }
}
