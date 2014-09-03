#include <Gosu/Gosu.hpp>
#include <SDL2/SDL.h>

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
    
    SDL_DisplayMode currentDisplayMode = { 0, 0 };
}

unsigned Gosu::screenWidth()
{
    // TODO - not thread-safe
    if (currentDisplayMode.w == 0) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GetDisplayMode(0, 0, &currentDisplayMode);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    return currentDisplayMode.w;
}

unsigned Gosu::screenHeight()
{
    // TODO - not thread-safe
    if (currentDisplayMode.h == 0) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GetDisplayMode(0, 0, &currentDisplayMode);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    return currentDisplayMode.h;
}

// Pessimistic fallback implementation for availableWidth / availableHeight.

#if !defined(GOSU_IS_MAC)
unsigned Gosu::availableWidth()
{
    return Gosu::screenWidth() * 0.9;
}

unsigned Gosu::availableHeight()
{
    return Gosu::screenHeight() * 0.8;
}
#endif
