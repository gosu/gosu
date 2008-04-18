#include <Gosu/Window.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Audio.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>

struct Gosu::Window::Impl
{
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Input> input;
    boost::scoped_ptr<Audio> audio;

    ::Window window;
    ::Display* dpy;
    ::XVisualInfo* vi;
    ::GLXContext cx;
    ::Colormap cmap;
    ::Atom deleteAtom;
    std::wstring title;
    unsigned int width, height;
    double updateInterval;
    bool showing, visible;

    bool isFullscreen;
    XF86VidModeModeInfo oldMode;

    void enterFullscreen();
    void doTick(Window* window);
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
        double updateInterval)
    : pimpl(new Impl)
{
    pimpl->dpy = XOpenDisplay(NULL);
    if (!pimpl->dpy)
        throw std::runtime_error("Cannot find display");

    static int attributeListSgl[] =
    {
        GLX_RGBA, GLX_DOUBLEBUFFER, // double buffering (?)
        GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
        GLX_DEPTH_SIZE, 1, None
    };

    pimpl->vi = glXChooseVisual(pimpl->dpy, DefaultScreen(pimpl->dpy),
        attributeListSgl);

    pimpl->cx = glXCreateContext(pimpl->dpy, pimpl->vi, 0,   GL_TRUE);

    pimpl->cmap = XCreateColormap(pimpl->dpy, RootWindow(pimpl->dpy, pimpl->vi->screen),
        pimpl->vi->visual, AllocNone);

    pimpl->deleteAtom = XInternAtom(pimpl->dpy, "WM_DELETE_WINDOW", true);
//    pimpl->blackColor = BlackPixel(pimpl->dpy, DefaultScreen(pimpl->dpy));
    pimpl->width = width;
    pimpl->height = height;
    pimpl->showing = false;
    pimpl->title = L"Gosu Window";
    pimpl->updateInterval = updateInterval;
    pimpl->visible = true;
    pimpl->isFullscreen = fullscreen;

    pimpl->input.reset(new Gosu::Input(pimpl->dpy));
    input().onButtonDown = boost::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = boost::bind(&Window::buttonUp, this, _1);


    // ******************** from show(): ********************

    XSetWindowAttributes swa;

    swa.colormap = pimpl->cmap;
    swa.border_pixel = 0;

    pimpl->window = XCreateWindow(pimpl->dpy, RootWindow(pimpl->dpy, pimpl->vi->screen), 0,
        0, pimpl->width, pimpl->height, 0, pimpl->vi->depth, InputOutput, pimpl->vi->visual,
        CWBorderPixel|CWColormap/*|CWEventMask*/, &swa);

    XSelectInput(pimpl->dpy, pimpl->window,
        StructureNotifyMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask /*| ClientMessageMask*/);

    // We want a [X]-Button
    if (pimpl->deleteAtom)
        XSetWMProtocols(pimpl->dpy, pimpl->window, &(pimpl->deleteAtom), 1);

    // We don't want the window to be resized.
    XSizeHints* sizeHints = XAllocSizeHints();
    sizeHints->flags = PMinSize | PMaxSize;
    sizeHints->min_width = sizeHints->max_width = pimpl->width;
    sizeHints->min_height = sizeHints->max_height = pimpl->height;
    XSetWMNormalHints(pimpl->dpy, pimpl->window, sizeHints);
    XFree(sizeHints);

    Atom stateAtom = XInternAtom(pimpl->dpy, "_NET_WM_STATE", False);
    Atom fullscreenAtom = XInternAtom(pimpl->dpy, "_NET_WM_STATE_FULLSCREEN", False);
    if (fullscreen)// && stateAtom != None && fullscreenAtom != None)
    {
        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.type = ClientMessage;
        xev.xclient.window = pimpl->window;
        xev.xclient.message_type = stateAtom;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 1;
        xev.xclient.data.l[1] = fullscreenAtom;
        xev.xclient.data.l[2] = 0;
        XSendEvent(pimpl->dpy, DefaultRootWindow(pimpl->dpy), False,
            SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    }

//    XMapWindow(pimpl->dpy, pimpl->window);
    glXMakeCurrent(pimpl->dpy, pimpl->window, pimpl->cx);

    pimpl->graphics.reset(new Gosu::Graphics(pimpl->width, pimpl->height, false));
    
    XColor black, dummy;
    XAllocNamedColor(pimpl->dpy, pimpl->cmap, "black", &black, &dummy);
    
    char emptyData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    Pixmap emptyBitmap =
        XCreateBitmapFromData(pimpl->dpy, pimpl->window, emptyData, 8, 8);
    Cursor emptyCursor = XCreatePixmapCursor(pimpl->dpy, emptyBitmap,
        emptyBitmap, &black, &black, 0, 0);
    XDefineCursor(pimpl->dpy, pimpl->window, emptyCursor);
    XFreeCursor(pimpl->dpy, emptyCursor);
}

Gosu::Window::~Window()
{
    XDestroyWindow(pimpl->dpy, pimpl->window);
    XSync(pimpl->dpy, false);
}

std::wstring Gosu::Window::caption() const
{
    // TODO: Update to _NET_WM_NAME

    return pimpl->title;
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    // TODO: Update to _NET_WM_NAME

    pimpl->title = caption;

    if(!pimpl->showing) return;

    std::string tmpString = narrow(pimpl->title);
    std::vector<char> title(pimpl->title.size() + 1);
    std::copy(tmpString.begin(), tmpString.end(), title.begin());
    title.back() = 0;

    XTextProperty titleprop;
    char* titlePtr = &title[0];
    XStringListToTextProperty(&titlePtr, 1, &titleprop);

    XSetWMName(pimpl->dpy, pimpl->window, &titleprop);
    XFree(titleprop.value);
    XSync(pimpl->dpy, false);
}

void Gosu::Window::Impl::enterFullscreen()
{
    // save old mode.
    XF86VidModeModeLine* l = (XF86VidModeModeLine*)((char*)&oldMode + sizeof oldMode.dotclock);
    XF86VidModeGetModeLine(dpy, XDefaultScreen(dpy), (int*)&oldMode.dotclock, l);

    // search fitting new mode
    int modeCnt;
    XF86VidModeModeInfo** modes;
    bool ret = XF86VidModeGetAllModeLines(dpy, XDefaultScreen(dpy), &modeCnt, &modes);

    if(!ret) throw std::runtime_error("Can't retrieve XF86 modes, fullscreen impossible.");

    bool switched = false;
    for(int i = 0; i < modeCnt; i++)
    {
        if(modes[i]->hdisplay == width && modes[i]->vdisplay == height)
        {
            XF86VidModeSwitchToMode(dpy, XDefaultScreen(dpy), modes[i]);
            switched = true;
            break;
        }
    }

    if(switched)
    {
        //XFlush(pimpl->dpy);
        //XSync(pimpl->dpy, 0);
        XMoveWindow(dpy, window, 0, 0);
        XRaiseWindow(dpy, window);
        XGrabPointer(dpy, window, true, 0, GrabModeAsync, GrabModeAsync, window,
            None, CurrentTime);
        XMapRaised(dpy, window);
        XSync(dpy, window);
        XWarpPointer(dpy, None, window, 0,0,0,0, 0, 0);
        XSync(dpy, window);
        XWarpPointer(dpy, None, window, 0,0,0,0, width/2, height/2);
    }
    else
    {
        XFree(modes);
        throw std::runtime_error("Can't find a fitting display mode, fullscreen not supported.");
    }

    isFullscreen = switched;

    XFree(modes);
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
    pimpl->showing = true;

    XMapWindow(pimpl->dpy, pimpl->window);
    XFlush(pimpl->dpy);

    if(pimpl->isFullscreen) pimpl->enterFullscreen();

    setCaption(pimpl->title);

    unsigned long startTime, endTime;
    while(pimpl->showing)
    {
        startTime = milliseconds();
        pimpl->doTick(this);
        if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
        endTime = milliseconds();

        if((endTime - startTime) < pimpl->updateInterval)
            sleep(pimpl->updateInterval - (endTime - startTime));
    }

    // Close the X window
    if(pimpl->isFullscreen)
    {
        XUngrabPointer(pimpl->dpy, CurrentTime);
        XF86VidModeSwitchToMode(pimpl->dpy, XDefaultScreen(pimpl->dpy), &(pimpl->oldMode));
    }

    XUnmapWindow(pimpl->dpy, pimpl->window);
    XSync(pimpl->dpy, false);
}

void Gosu::Window::close()
{
/*    if(pimpl->isFullscreen)
    {
        XUngrabPointer(pimpl->dpy, CurrentTime);
        XF86VidModeSwitchToMode(pimpl->dpy, XDefaultScreen(pimpl->dpy), &(pimpl->oldMode));
    }

    XUnmapWindow(pimpl->dpy, pimpl->window);
    XSync(pimpl->dpy, false);*/
    pimpl->showing = false;
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
        pimpl->audio.reset(new Gosu::Audio());
    return *pimpl->audio;
}

Gosu::Audio& Gosu::Window::audio()
{
    if (!pimpl->audio)
        pimpl->audio.reset(new Gosu::Audio());
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

namespace
{
    void makeCurrentContext(Display* dpy, GLXDrawable drawable, GLXContext context) {
        if (!glXMakeCurrent(dpy, drawable, context))
            printf("glXMakeCurrent failed\n");
    }

    void releaseContext(Display* dpy, GLXContext context) {
        glXDestroyContext(dpy, context);
    }
}

Gosu::Window::SharedContext Gosu::Window::createSharedContext() {
    const char* displayName = DisplayString( pimpl->dpy );
    Display* dpy2 = XOpenDisplay( displayName );
    if (!dpy2)
        throw std::runtime_error("Could not duplicate X display");
    
	GLXContext ctx = glXCreateContext(dpy2, pimpl->vi, pimpl->cx, True);
	if (!ctx)
        throw std::runtime_error("Could not create shared GLX context");
    
    return SharedContext(
        new boost::function<void()>(boost::bind(makeCurrentContext, dpy2, pimpl->window, ctx)),
        boost::bind(releaseContext, dpy2, ctx));
}

void Gosu::Window::Impl::doTick(Window* window)
{
    while(::XPending(dpy))
    {
        ::XEvent event;
        XNextEvent(dpy, &event);

        if(!window->input().feedXEvent(event, window))
        {
            if(event.type == VisibilityNotify)
            {
                if(event.xvisibility.state == VisibilityFullyObscured)
                    visible = false;
                else
                    visible = true;
            }
            else if(event.type == ConfigureNotify)
            {
            }
            else if(event.type == ClientMessage)
            {
                if(static_cast<unsigned>(event.xclient.data.l[0]) == deleteAtom)
                    window->close();
            }
        }
    }

    if(visible && window->graphics().begin(Colors::black))
    {
        window->draw();
        window->graphics().end();
        glXSwapBuffers(dpy, this->window);
    }

    window->input().update();
    window->update();
}
