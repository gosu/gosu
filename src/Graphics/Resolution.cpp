#include <Gosu/Gosu.hpp>

#ifdef GOSU_IS_WIN
#include <windows.h>
// Note: This does not use the SDL2 variant below, which reports 2560x1920 in my Windows VM, which is running at 2560x1080. Last checked with SDL 2.0.3.
// TODO: Try this on a real Windows machine.

unsigned Gosu::screenWidth()
{
    return GetSystemMetrics(SM_CXSCREEN);
}
    
unsigned Gosu::screenHeight()
{
    return GetSystemMetrics(SM_CYSCREEN);
}

// Note: It is okay for this couple of functions to be OS-specific.
namespace
{
    SIZE calculateAvailableSize()
    {
        RECT workArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

        RECT windowSize = workArea;
        // Note: This should be kept in sync with STYLE_NORMAL in SDL_windowswindow.c.
        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        AdjustWindowRectEx(&windowSize, style, FALSE, 0);

        SIZE availableSize;
        availableSize.cx = 2 * (workArea.right - workArea.left) - (windowSize.right - windowSize.left);
        availableSize.cy = 2 * (workArea.bottom - workArea.top) - (windowSize.bottom - windowSize.top);
        return availableSize;
    }

    SIZE availableSize = calculateAvailableSize();
}

unsigned Gosu::availableWidth()
{
    return availableSize.cx;
}

unsigned Gosu::availableHeight()
{
    return availableSize.cy;
}
#else
#include <SDL2/SDL.h>

namespace
{
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
#endif
