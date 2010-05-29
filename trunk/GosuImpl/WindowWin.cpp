#define NOMINMAX // RAGE >_<

#include <Gosu/Window.hpp>
#include <Gosu/WinUtility.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <boost/bind.hpp>
#include <cassert>
#include <stdexcept>
#include <vector>

// TODO: Put fullscreen logic in different file, track fullscreen state and
// enable dynamic toggling between fullscreen and window.

namespace Gosu
{
    HWND __Gosu_HWND_for_FMOD = 0;

    unsigned screenWidth()
    {
        return GetSystemMetrics(SM_CXSCREEN);
    }
    
    unsigned screenHeight()
    {
        return GetSystemMetrics(SM_CYSCREEN);
    }

    namespace
    {
        // Mode guessing experimentally adapted from GLFW library.
        // http://glfw.sourceforge.net/

        int findClosestVideoMode(int *w, int *h, int *bpp, int *refresh)
        {
            int     mode, bestmode, match, bestmatch, rr, bestrr, success;
            DEVMODE dm;

            // Find best match
            bestmatch = 0x7fffffff;
            bestrr    = 0x7fffffff;
            mode = bestmode = 0;
            do
            {
                dm.dmSize = sizeof(DEVMODE);
                success = EnumDisplaySettings(NULL, mode, &dm);
                if( success )
                {
                    match = dm.dmBitsPerPel - *bpp;
                    if( match < 0 ) match = -match;
                    match = (match << 25) |
                            ((dm.dmPelsWidth - *w) * (dm.dmPelsWidth - *w) +
                             (dm.dmPelsHeight - *h) * (dm.dmPelsHeight - *h));
                    if( match < bestmatch )
                    {
                        bestmatch = match;
                        bestmode  = mode;
                        bestrr = (dm.dmDisplayFrequency - *refresh) *
                                 (dm.dmDisplayFrequency - *refresh);
                    }
                    else if( match == bestmatch && *refresh > 0 )
                    {
                        rr = (dm.dmDisplayFrequency - *refresh) *
                             (dm.dmDisplayFrequency - *refresh);
                        if( rr < bestrr )
                        {
                            bestmatch = match;
                            bestmode  = mode;
                            bestrr    = rr;
                        }
                    }
                }
                ++mode;
            }
            while (success);

            // Get the parameters for the best matching display mode
            dm.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings( NULL, bestmode, &dm );

            *w = dm.dmPelsWidth;
            *h = dm.dmPelsHeight;
            *bpp = dm.dmBitsPerPel;
            *refresh = dm.dmDisplayFrequency;

            return bestmode;
        }

        void setVideoMode(int mode)
        {
            // Get the parameters for the best matching display mode
            DEVMODE dm;
            dm.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(NULL, mode, &dm);

            // Set which fields we want to specify
            dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

            // Change display setting
            dm.dmSize = sizeof(DEVMODE);
            if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                throw std::runtime_error("Could not set fullscreen mode");
        }

        void setupVSync()
        {
            char* extensions = (char*)glGetString(GL_EXTENSIONS);
			// The Intel BootCamp drivers will actually have a proc address for wglSwapInterval
            // that doesn't do much, so check the string instead of just getting the address.
            if (!strstr(extensions, "WGL_EXT_swap_control"))
                return;
            typedef void (APIENTRY *PFNWGLEXTSWAPCONTROLPROC) (int);
            PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT =
                (PFNWGLEXTSWAPCONTROLPROC) wglGetProcAddress("wglSwapIntervalEXT");
            if (!wglSwapIntervalEXT)
                return;
            wglSwapIntervalEXT(1);
        }

        LRESULT CALLBACK windowProc(HWND wnd, UINT message, WPARAM wparam,
            LPARAM lparam)
        {
            LONG_PTR lptr = GetWindowLongPtr(wnd, GWLP_USERDATA);

            if (lptr)
            {
                Window* obj = reinterpret_cast<Window*>(lptr);
                return obj->handleMessage(message, wparam, lparam);
            }
            else
                return DefWindowProc(wnd, message, wparam, lparam);
        }

        LPCTSTR windowClass()
        {
            static LPCTSTR name = 0;
            if (name)
                return name;
            
            WNDCLASSEX wc;
            ZeroMemory(&wc, sizeof wc);
            wc.cbSize = sizeof wc;
            wc.lpszClassName = L"Gosu::Window";
            wc.style = CS_OWNDC;
            wc.lpfnWndProc = windowProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = Win::instance();
            wc.hIcon = ExtractIcon(wc.hInstance, Win::appFilename().c_str(), 0);
            wc.hCursor = 0;
            wc.hbrBackground = CreateSolidBrush(0);
            wc.lpszMenuName = 0;
            wc.hIconSm = (HICON)CopyImage(wc.hIcon, IMAGE_ICON,
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                LR_COPYFROMRESOURCE | LR_COPYRETURNORG);

            
            name = reinterpret_cast<LPCTSTR>(RegisterClassEx(&wc));
            Win::check(name, "registering a window class");
            return name;
        }
    }
}

struct Gosu::Window::Impl
{
    HWND handle;
	HDC hdc;
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Input> input;
	double updateInterval;
    bool iconified;

    unsigned originalWidth, originalHeight;

    Impl()
    : handle(0), hdc(0), iconified(false)
    {
    }

    ~Impl()
    {
        if (hdc)
            ReleaseDC(handle, hdc);
        if (handle)
            DestroyWindow(handle);
    }
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
    double updateInterval)
: pimpl(new Impl)
{
    pimpl->originalWidth = width;
    pimpl->originalHeight = height;
    
    // Select window styles depending on mode.fullscreen.
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    DWORD styleEx = WS_EX_APPWINDOW;
    if (fullscreen)
    {
        style |= WS_POPUP;
#ifdef NDEBUG
        styleEx |= WS_EX_TOPMOST;
#endif
    }
    else
    {
        style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        styleEx |= WS_EX_WINDOWEDGE;
    }

    pimpl->handle = CreateWindowEx(styleEx, windowClass(), 0, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        Win::instance(), 0);
    Win::check(pimpl->handle);
    __Gosu_HWND_for_FMOD = pimpl->handle;

	pimpl->hdc = GetDC(handle());
    Win::check(pimpl->hdc);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof pfd);
    pfd.nSize        = sizeof pfd;
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iLayerType   = PFD_MAIN_PLANE;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
	int pf = ChoosePixelFormat(pimpl->hdc, &pfd);
    Win::check(pf);
    Win::check(SetPixelFormat(pimpl->hdc, pf, &pfd));

	HGLRC hrc = Win::check(wglCreateContext(pimpl->hdc), "creating rendering context");
    Win::check(wglMakeCurrent(pimpl->hdc, hrc), "selecting the rendering context");

    setupVSync();

    SetLastError(0);
    SetWindowLongPtr(handle(), GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));
    if (GetLastError() != 0)
        Win::throwLastError("setting the window's GWLP_USERDATA pointer");

    // Windowed: Create window large enough to display stuff
    // This is a pretty brutal heuristic I guess.
    
    if (!fullscreen)
    {
		double factor = std::min(0.9 * screenWidth() / width,
								 0.8 * screenHeight() / height);
	    
		if (factor < 1)
			width *= factor, height *= factor;
	}

    // Determine the size the window needs to have.
    RECT rc = { 0, 0, width, height };
    AdjustWindowRectEx(&rc, style, FALSE, styleEx);
    unsigned windowW = rc.right - rc.left;
    unsigned windowH = rc.bottom - rc.top;

    int windowX = 0;
    int windowY = 0;

    if (!fullscreen)
    {
        // Center the window.
        HWND desktopWindow = GetDesktopWindow();
        RECT desktopRect;
        GetClientRect(desktopWindow, &desktopRect);
        int desktopW = desktopRect.right - desktopRect.left;
        int desktopH = desktopRect.bottom - desktopRect.top;
        windowX = (desktopW - windowW) / 2;
        windowY = (desktopH - windowH) / 2;
    }

    MoveWindow(handle(), windowX, windowY, windowW, windowH, false);

    pimpl->graphics.reset(new Gosu::Graphics(width, height, fullscreen));
	graphics().setResolution(pimpl->originalWidth, pimpl->originalHeight);
    pimpl->input.reset(new Gosu::Input(handle()));
	input().setMouseFactors(1.0 * pimpl->originalWidth / width, 1.0 * pimpl->originalHeight / height);
    input().onButtonDown = boost::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = boost::bind(&Window::buttonUp, this, _1);

    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    wglMakeCurrent(0, 0);
    __Gosu_HWND_for_FMOD = 0;
}

std::wstring Gosu::Window::caption() const
{
    int bufLen = GetWindowTextLength(handle()) + 1;

    if (bufLen < 2)
        return L"";

    std::vector<TCHAR> buf(bufLen);
    GetWindowText(handle(), &buf.front(), bufLen);
    return &buf.front();
}

void Gosu::Window::setCaption(const std::wstring& value)
{
    SetWindowText(handle(), value.c_str());
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

namespace GosusDarkSide
{
    // TODO: Find a way for this to fit into Gosu's design.
    // This can point to a function that wants to be called every
    // frame, e.g. rb_thread_schedule.
    typedef void (*HookOfHorror)();
    HookOfHorror oncePerTick = 0;
}

void Gosu::Window::show()
{
    int w = pimpl->originalWidth, h = pimpl->originalHeight, bpp = 32, rr = 60;
    if (graphics().fullscreen())
        setVideoMode(findClosestVideoMode(&w, &h, &bpp, &rr));
    ShowWindow(handle(), SW_SHOW);
    UpdateWindow(handle());
    try
    {
        Win::processMessages();

		unsigned lastTick = 0;

		for (;;)
	    {
			Win::processMessages();

			// Whyever this would happen...!
			if (!::IsWindowVisible(handle()))
			{
				Sleep(50);
				return;
			}

			unsigned ms = milliseconds();

			if (ms < lastTick || ms - lastTick >= static_cast<unsigned>(pimpl->updateInterval))
			{
				lastTick = ms;
				input().update();
			    update();
                if (needsRedraw())
      				::InvalidateRect(handle(), 0, FALSE);
				// There probably should be a proper "oncePerTick" handler
				// system in the future. Right now, this is necessary to give
				// timeslices to Ruby's green threads in Ruby/Gosu.
		        if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
			}
            else if (pimpl->updateInterval - (ms - lastTick) > 5)
				// More than 5 ms left until next update: Sleep to reduce
				// processur usage, Sleep() is accurate enough for that.
				Sleep(5);
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
    ShowWindow(handle(), SW_HIDE);
    if (graphics().fullscreen())
        ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
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
    if (message == WM_SETFOCUS && graphics().fullscreen() && IsWindowVisible(pimpl->handle))
    {
        if (pimpl->iconified)
        {
            OpenIcon(pimpl->handle);
            int w = graphics().width(), h = graphics().height(), bpp = 32, rr = 60;
            setVideoMode(findClosestVideoMode(&w, &h, &bpp, &rr));
            pimpl->iconified = false;
        }
        return 0;
    }

    if (message == WM_KILLFOCUS && graphics().fullscreen() && IsWindowVisible(pimpl->handle))
    {
        if (!pimpl->iconified)
        {
            ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
            CloseWindow(pimpl->handle);
            pimpl->iconified = true;
        }
        return 0;
    }

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
        SwapBuffers(pimpl->hdc);
        ValidateRect(handle(), 0);
        return 0;
    }

    if (message == WM_SETCURSOR && LOWORD(lparam) == HTCLIENT)
    {
        SetCursor(0);
        return TRUE;
    }

    if (message == WM_SYSCOMMAND)
    {
        switch(wparam)
        {
            case SC_SCREENSAVE:
            case SC_MONITORPOWER:
                if (graphics().fullscreen())
                    return 0;
                else
                    break;
            case SC_KEYMENU:
                return 0;
        }
    }

    if (pimpl->input && input().textInput() && input().textInput()->feedMessage(message, wparam, lparam))
    {
        return 0;
    }

    return DefWindowProc(handle(), message, wparam, lparam);
}

// Deprecated.

class Gosu::Audio {};
namespace { Gosu::Audio dummyAudio; }

const Gosu::Audio& Gosu::Window::audio() const
{
    return dummyAudio;
}
 
Gosu::Audio& Gosu::Window::audio()
{
    return dummyAudio;
}
