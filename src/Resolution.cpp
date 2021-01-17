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

static NSSize max_window_size(Gosu::Window* window)
{
    // Keep in sync with SDL_cocoawindow.m.
    auto style = NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask;

    auto index = window ? SDL_GetWindowDisplayIndex(Gosu::shared_window()) : 0;
    auto screen_frame = NSScreen.screens[index].visibleFrame;
    return [NSWindow contentRectForFrameRect:screen_frame styleMask:style].size;
}

int Gosu::available_width(Window* window)
{
    return max_window_size(window).width;
}

int Gosu::available_height(Window* window)
{
    return max_window_size(window).height;
}
#endif

#ifdef GOSU_IS_WIN
#include <windows.h>
#include <SDL_syswm.h>

static SIZE max_window_size(Gosu::Window* window)
{
    RECT work_area;
    
    if (window == nullptr) {
        // Easy case: Return the work area of the primary monitor.
        SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0);
    }
    else {
        // Return the work area of the monitor the window is on.
        SDL_SysWMinfo wm_info;
        SDL_VERSION(&wm_info.version);
        SDL_GetWindowWMInfo(Gosu::shared_window(), &wm_info);
        HMONITOR monitor = MonitorFromWindow(wm_info.info.win.window, MONITOR_DEFAULTTONEAREST);
        
        MONITORINFO monitor_info;
        monitor_info.cbSize = sizeof(monitor_info);
        GetMonitorInfo(monitor, &monitor_info);
        work_area = monitor_info.rcWork;
    }
    
    RECT window_size = work_area;
    // Keep in sync with STYLE_NORMAL in SDL_windowswindow.c.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&window_size, style, FALSE, 0);
    
    // Because AdjustWindowRectEx will make our rect larger, not smaller, we need to perform some
    // unintuitive math here.
    SIZE size;
    size.cx = 2 * (work_area.right - work_area.left) - (window_size.right - window_size.left);
    size.cy = 2 * (work_area.bottom - work_area.top) - (window_size.bottom - window_size.top);
    return size;
}

int Gosu::available_width(Window* window)
{
    return max_window_size(window).cx;
}

int Gosu::available_height(Window* window)
{
    return max_window_size(window).cy;
}
#endif

#ifdef GOSU_IS_X
// Pessimistic fallback implementation for available_width / available_height.
// TODO: Look at this NET_WORKAREA based implementation: https://github.com/glfw/glfw/pull/989/files
int Gosu::available_width(Window* window)
{
    return static_cast<unsigned>(Gosu::screen_width(window) * 0.9);
}

int Gosu::available_height(Window* window)
{
    return static_cast<unsigned>(Gosu::screen_height(window) * 0.8);
}
#endif

#endif
