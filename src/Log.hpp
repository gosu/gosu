#pragma once

#include <cstdlib>
#include <utility>

namespace Gosu
{
    template<class... Args>
    void log(const char* format, Args&&... args)
    {
        using namespace std;
        
        if (getenv("GOSU_DEBUG")) {
            fprintf(stderr, format, std::forward<Args>(args)...);
            fprintf(stderr, "\n");
        }
    }
}

