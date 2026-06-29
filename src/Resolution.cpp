#include <Gosu/Platform.hpp>
#ifndef GOSU_IS_IPHONE // Definitions for iOS live in WindowUIKit.cpp

#include <Gosu/Window.hpp>
#include "GraphicsImpl.hpp"
#include <SDL3/SDL.h>

static const SDL_DisplayMode& display_mode(const Gosu::Window* window)
{
    static const struct VideoSubsystem : Gosu::Noncopyable
    {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    SDL_DisplayID index
        = window ? SDL_GetDisplayForWindow(window->sdl_window()) : SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* result = SDL_GetDesktopDisplayMode(index);
    if (result == nullptr) {
        throw std::runtime_error("SDL_GetDesktopDisplayMode: " + std::string(SDL_GetError()));
    }
    return *result;
}

int Gosu::screen_width(const Window* window)
{
    return display_mode(window).w;
}

int Gosu::screen_height(const Window* window)
{
    return display_mode(window).h;
}

#ifdef GOSU_IS_MAC
#import <AppKit/AppKit.h>

static SDL_Rect max_window_size(const Gosu::Window* window)
{
    static const struct VideoSubsystem : Gosu::Noncopyable
    {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    // The extra size that a window needs depends on its style.
    // This logic must be kept in sync with SDL_cocoawindow.m to be 100% accurate.
    NSUInteger style
        = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

    // SDL_GetDisplayUsableBounds gives the maximum size of a borderless window (the screen minus
    // the menu bar and dock).
    SDL_DisplayID display
        = window ? SDL_GetDisplayForWindow(window->sdl_window()) : SDL_GetPrimaryDisplay();
    SDL_Rect result;
    if (!SDL_GetDisplayUsableBounds(display, &result)) {
        throw std::runtime_error("SDL_GetDisplayUsableBounds: " + std::string(SDL_GetError()));
    }

    // Now subtract the decorations that a default-styled Cocoa window would add.
    NSRect content =
        [NSWindow contentRectForFrameRect:NSMakeRect(0, 0, result.w, result.h) styleMask:style];
    result.w = content.size.width;
    result.h = content.size.height;
    return result;
}
#endif

#ifdef GOSU_IS_WIN
#include <dwmapi.h>
#include <windows.h>

static SDL_Rect max_window_size(const Gosu::Window* window)
{
    // Replicate SDL's WIN_GetWindowBordersSize implementation because the original includes the
    // size of the drop shadow, which doesn't make sense as they don't actually take up space.
    // REF: https://github.com/libsdl-org/SDL/issues/3835

    static const struct VideoSubsystem : Gosu::Noncopyable
    {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    SDL_DisplayID display
        = window ? SDL_GetDisplayForWindow(window->sdl_window()) : SDL_GetPrimaryDisplay();
    SDL_Rect rect;
    if (!SDL_GetDisplayUsableBounds(display, &rect)) {
        throw std::runtime_error("SDL_GetDisplayUsableBounds: " + std::string(SDL_GetError()));
    }

    // Note: We only subtract the window border when a window is given - but the main use case of
    // available_width/available_height is to determine the available space for a new window!
    // TODO: Replace DWM with a better API (AdjustWindowRectExForDpi?)
    if (window) {
        HWND hwnd = static_cast<HWND>(
            SDL_GetPointerProperty(SDL_GetWindowProperties(window->sdl_window()),
                                   SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

        RECT rcClient, rcWindow;
        POINT ptDiff;
        int top = 0, left = 0, bottom = 0, right = 0;

        /* rcClient stores the size of the inner window, while rcWindow stores the outer size relative to the top-left
         * screen position; so the top/left values of rcClient are always {0,0} and bottom/right are {height,width} */
        GetClientRect(hwnd, &rcClient);
        DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rcWindow, sizeof(rcWindow));

        /* convert the top/left values to make them relative to
         * the window; they will end up being slightly negative */
        ptDiff.y = rcWindow.top;
        ptDiff.x = rcWindow.left;

        ScreenToClient(hwnd, &ptDiff);

        rcWindow.top = ptDiff.y;
        rcWindow.left = ptDiff.x;

        /* convert the bottom/right values to make them relative to the window,
         * these will be slightly bigger than the inner width/height */
        ptDiff.y = rcWindow.bottom;
        ptDiff.x = rcWindow.right;

        ScreenToClient(hwnd, &ptDiff);

        rcWindow.bottom = ptDiff.y;
        rcWindow.right = ptDiff.x;

        /* Now that both the inner and outer rects use the same coordinate system we can substract them to get the border size.
         * Keep in mind that the top/left coordinates of rcWindow are negative because the border lies slightly before {0,0},
         * so switch them around because SDL2 wants them in positive. */
        top = rcClient.top - rcWindow.top;
        left = rcClient.left - rcWindow.left;
        bottom = rcWindow.bottom - rcClient.bottom;
        right = rcWindow.right - rcClient.right;

        rect.w -= left + right;
        rect.h -= top + bottom;
    }

    // Return a rect to have one less Gosu::available_width/height implementation.
    return rect;
}
#endif

#ifdef GOSU_IS_X
static SDL_Rect max_window_size(const Gosu::Window* window)
{
    static const struct VideoSubsystem : Gosu::Noncopyable
    {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    SDL_DisplayID index
        = window ? SDL_GetDisplayForWindow(window->sdl_window()) : SDL_GetPrimaryDisplay();
    SDL_Rect rect;
    if (!SDL_GetDisplayUsableBounds(index, &rect)) {
        throw std::runtime_error("SDL_GetDisplayUsableBounds: " + std::string(SDL_GetError()));
    }

    if (window) {
        int top, left, bottom, right;
        SDL_GetWindowBordersSize(window->sdl_window(), &top, &left, &bottom, &right);
        rect.w -= left + right;
        rect.h -= top + bottom;
    }

    return rect;
}
#endif

int Gosu::available_width(const Window* window)
{
    return max_window_size(window).w;
}

int Gosu::available_height(const Window* window)
{
    return max_window_size(window).h;
}

#endif
