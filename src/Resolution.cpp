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

#ifdef GOSU_IS_MAC
#import <AppKit/AppKit.h>

static SDL_Rect max_window_size(Gosu::Window* window)
{
    // The extra size that a window needs depends on its style.
    // This logic must be kept in sync with SDL_cocoawindow.m to be 100% accurate.
    NSUInteger style;
    if (window && window->borderless()) {
        style = NSWindowStyleMaskBorderless;
    }
    else {
        style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    }
    if (window && window->resizable()) {
        style |= NSWindowStyleMaskResizable;
    }

    auto index = window ? SDL_GetWindowDisplayIndex(Gosu::shared_window()) : 0;
    NSRect screen_frame = NSScreen.screens[index].visibleFrame;
    NSRect content_rect = [NSWindow contentRectForFrameRect:screen_frame styleMask:style];

    SDL_Rect result;
    result.x = 0;
    result.y = 0;
    result.w = content_rect.size.width;
    result.h = content_rect.size.height;
    return result;
}
#endif

// TODO: Remove this implementation and remove ifdef for GOSU_IS_X once WIN_GetWindowBordersSize is patched
#ifdef GOSU_IS_WIN
#include <windows.h>
#include <SDL_syswm.h>
#include <dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

static SDL_Rect max_window_size(Gosu::Window* window)
{
    // Replicate SDL's WIN_GetWindowBordersSize implementation (https://github.com/libsdl-org/SDL/blob/9f71a809e9bd6fbb5fa401a45c1537fc26abc1b4/src/video/windows/SDL_windowswindow.c#L514-L554)
    // until it's patched to ignore the window drop shadow (window border is 1px but with drop shadow it's reported as 8px)
    // REF: https://github.com/libsdl-org/SDL/issues/3835

    static struct VideoSubsystem {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    int index = window ? SDL_GetWindowDisplayIndex(Gosu::shared_window()) : 0;
    SDL_Rect rect;
    SDL_GetDisplayUsableBounds(index, &rect);

    if (window) {
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        SDL_GetWindowWMInfo(Gosu::shared_window(), &info);
        HWND hwnd = info.info.win.window;

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
static SDL_Rect max_window_size(Gosu::Window* window)
{
    static struct VideoSubsystem {
        VideoSubsystem() { SDL_InitSubSystem(SDL_INIT_VIDEO); };
        ~VideoSubsystem() { SDL_QuitSubSystem(SDL_INIT_VIDEO); };
    } subsystem;

    int index = window ? SDL_GetWindowDisplayIndex(Gosu::shared_window()) : 0;
    SDL_Rect rect;
    int top, left, bottom, right;
    SDL_GetDisplayUsableBounds(index, &rect);
    SDL_GetWindowBordersSize(Gosu::shared_window(), &top, &left, &bottom, &right);

    rect.w -= left + right;
    rect.h -= top + bottom;

    return rect;
}
#endif

int Gosu::available_width(Window* window)
{
    return max_window_size(window).w;
}

int Gosu::available_height(Window* window)
{
    return max_window_size(window).h;
}
#endif
