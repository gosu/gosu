#include <Gosu/Platform.hpp>
#ifndef GOSU_IS_IPHONE // iPhone definitions live in WindowUIKit.cpp

#include <Gosu/Window.hpp>
#include "GraphicsImpl.hpp"
#include <SDL.h>

static SDL_DisplayMode display_mode(Gosu::Window* window)
{
    static struct VideoSubsystem {
        VideoSubsystem()  { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    int index = window ? SDL_GetWindowDisplayIndex(Gosu::shared_window()) : 0;
    SDL_DisplayMode result;
    SDL_GetDesktopDisplayMode(index, &result);
    return result;
}

int Gosu::screen_width(Window* window)
{
    return display_mode(window).w;
}

int Gosu::screen_height(Window* window)
{
    return display_mode(window).h;
}

static SDL_Rect max_window_size(Gosu::Window* window)
{
    static struct VideoSubsystem {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    int index = window ? SDL_GetWindowDisplayIndex(Gosu::shared_window()) : 0;
    SDL_Rect rect;
    SDL_GetDisplayUsableBounds(index, &rect);
    return rect;
}

int Gosu::available_width(Window* window)
{
    return max_window_size(window).w;
}

int Gosu::available_height(Window* window)
{
    return max_window_size(window).h;
}
#endif