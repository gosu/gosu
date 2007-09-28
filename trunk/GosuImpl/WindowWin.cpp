#include <Gosu/Window.hpp>
#include <Gosu/WinUtility.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/Input.hpp>
#include <boost/bind.hpp>
#include <cassert>
#include <stdexcept>
#include <vector>

namespace Gosu
{
    namespace
    {
        LRESULT CALLBACK windowProc(HWND wnd, UINT message, WPARAM wparam,
            LPARAM lparam)
        {
            LONG_PTR lptr = ::GetWindowLongPtr(wnd, GWLP_USERDATA);

            if (lptr)
            {
                Window* obj = reinterpret_cast<Window*>(lptr);
                return obj->handleMessage(message, wparam, lparam);
            }
            else
                return ::DefWindowProc(wnd, message, wparam, lparam);
        }

        LPCTSTR windowClass()
        {
            static LPCTSTR name = 0;
            if (name)
                return name;

            WNDCLASS wc;
            wc.lpszClassName = L"Gosu::Window";
            wc.style = CS_CLASSDC;
            wc.lpfnWndProc = windowProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = Win::instance();
            wc.hIcon = 0;
            wc.hCursor = 0;
            wc.hbrBackground = 0;
            wc.lpszMenuName = 0;
            
            name = reinterpret_cast<LPCTSTR>(::RegisterClass(&wc));
            Win::check(name, "registering a window class");
            return name;
        }
    }
}

struct Gosu::Window::Impl
{
    HWND handle;
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Audio> audio;
    boost::scoped_ptr<Input> input;
	unsigned updateInterval;

    Impl()
    : handle(0)
    {
    }

    ~Impl()
    {
        if (handle)
            ::DestroyWindow(handle);
    }
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
    unsigned updateInterval)
: pimpl(new Impl)
{
    // Don't even get started when there's no suitable display mode.
    boost::optional<DisplayMode> mode =
        DisplayMode::find(width, height, fullscreen);
    if (!mode)
        throw std::runtime_error("No suitable display mode found");

    // Select window styles depending on mode.fullscreen.
    DWORD style = 0;
    DWORD styleEx = WS_EX_APPWINDOW;
    if (fullscreen)
    {
        style |= WS_POPUP;
        styleEx |= WS_EX_TOPMOST;
    }
    else
    {
        style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        styleEx |= WS_EX_WINDOWEDGE;
    }

    pimpl->handle = ::CreateWindowEx(styleEx, windowClass(), 0, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        Win::instance(), 0);

    ::SetLastError(0);
    ::SetWindowLongPtr(handle(), GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));
    if (::GetLastError() != 0)
        Win::throwLastError("setting the window's GWLP_USERDATA pointer");

    // Determine the size the window needs to have.
    RECT rc = { 0, 0, width, height };
    ::AdjustWindowRectEx(&rc, style, FALSE, styleEx);
    unsigned windowW = rc.right - rc.left;
    unsigned windowH = rc.bottom - rc.top;

    // Center the window.
    HWND desktopWindow = ::GetDesktopWindow();
    RECT desktopRect;
    ::GetClientRect(desktopWindow, &desktopRect);
    int desktopW = desktopRect.right - desktopRect.left;
    int desktopH = desktopRect.bottom - desktopRect.top;
    unsigned windowX = (desktopW - windowW) / 2;
    unsigned windowY = (desktopH - windowH) / 2;

    ::MoveWindow(handle(), windowX, windowY, windowW, windowH, false);

    RECT outputRect = { 0, 0, width, height };
    pimpl->graphics.reset(new Gosu::Graphics(handle(), outputRect, *mode));
    pimpl->input.reset(new Gosu::Input(handle()));
    input().onButtonDown = boost::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = boost::bind(&Window::buttonUp, this, _1);

    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    ::KillTimer(handle(), 1337);
}

std::wstring Gosu::Window::caption() const
{
    int bufLen = ::GetWindowTextLength(handle()) + 1;

    if (bufLen < 2)
        return L"";

    std::vector<TCHAR> buf(bufLen);
    ::GetWindowText(handle(), &buf.front(), bufLen);
    return &buf.front();
}

void Gosu::Window::setCaption(const std::wstring& value)
{
    ::SetWindowText(handle(), value.c_str());
}

void Gosu::Window::show()
{
    ::ShowWindow(handle(), SW_SHOW);
    try
    {
        Win::processMessages();

		unsigned lastTick = 0;
		bool lastFrameSkipped = false;

		for (;;) 
	    {
			Win::processMessages();

			if (!::IsWindowVisible(handle()))
				return;

			unsigned ms = milliseconds();

			if (ms >= lastTick + pimpl->updateInterval)
			{
				lastTick = ms;
				input().update();
			    update();
				::InvalidateRect(handle(), 0, FALSE), lastFrameSkipped = false;
			}
		}
    }
    catch (...)
    {
        close();
        throw;
    }
}

void Gosu::Window::close()
{
    ::ShowWindow(handle(), SW_HIDE);
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
}

const Gosu::Audio& Gosu::Window::audio() const
{
    if (!pimpl->audio)
        pimpl->audio.reset(new Gosu::Audio(handle()));
    return *pimpl->audio;
}

Gosu::Audio& Gosu::Window::audio()
{
    if (!pimpl->audio)
        pimpl->audio.reset(new Gosu::Audio(handle()));
    return *pimpl->audio;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *pimpl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *pimpl->input;
}

HWND Gosu::Window::handle() const
{
    return pimpl->handle;
}

LRESULT Gosu::Window::handleMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_CLOSE)
    {
        close();
        return 0;
    }

    if (message == WM_PAINT)
    {
        if (pimpl->graphics && graphics().begin())
        {
            try
            {
                draw();
            }
            catch (...)
            {
                graphics().end();
                throw;
            }
            graphics().end();
        }
        ::ValidateRect(handle(), 0);
        return 0;
    }

    if (message == WM_SETCURSOR && LOWORD(lparam) == HTCLIENT)
    {
        ::SetCursor(0);
        return TRUE;
    }

    return ::DefWindowProc(handle(), message, wparam, lparam);
}
