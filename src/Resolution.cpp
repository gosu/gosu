#include <Gosu/Gosu.hpp>

#ifdef GOSU_IS_WIN
#include <windows.h>

// On Windows, do not use the SDL 2 code below. It reports 2560x1920 in my Windows VM, which is
// running at 2560x1080. (SDL 2.0.3)

unsigned Gosu::screen_width()
{
    return GetSystemMetrics(SM_CXSCREEN);
}

unsigned Gosu::screen_height()
{
    return GetSystemMetrics(SM_CYSCREEN);
}

static SIZE calculate_available_size()
{
    RECT work_area;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
    
    RECT window_size = work_area;
    // Note: This should be kept in sync with STYLE_NORMAL in SDL_windowswindow.c.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&window_size, style, FALSE, 0);
    
    SIZE size;
    size.cx = 2 * (work_area.right - work_area.left) - (window_size.right - window_size.left);
    size.cy = 2 * (work_area.bottom - work_area.top) - (window_size.bottom - window_size.top);
    return size;
}

static SIZE available_size = calculate_available_size();

unsigned Gosu::available_width()
{
    return available_size.cx;
}

unsigned Gosu::available_height()
{
    return available_size.cy;
}
#elif !defined(GOSU_IS_IPHONE)
#include <SDL.h>

static SDL_DisplayMode current_display_mode = { 0, 0 };

unsigned Gosu::screen_width()
{
    // TODO - not thread-safe
    if (current_display_mode.w == 0) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GetDisplayMode(0, 0, &current_display_mode);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    return current_display_mode.w;
}

unsigned Gosu::screen_height()
{
    // TODO - not thread-safe
    if (current_display_mode.h == 0) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GetDisplayMode(0, 0, &current_display_mode);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    return current_display_mode.h;
}

// Pessimistic fallback implementation for available_width / available_height.

#if !defined(GOSU_IS_MAC)
unsigned Gosu::available_width()
{
    return static_cast<unsigned>(Gosu::screen_width() * 0.9);
}

unsigned Gosu::available_height()
{
    return static_cast<unsigned>(Gosu::screen_height() * 0.8);
}
#endif
#endif
