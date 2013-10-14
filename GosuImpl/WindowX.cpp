// While (re)writing this file, I have been looking at many other libraries since all the
// "official" documentation was horrible, at least those parts that I was able to find.
// Kudos to the Pyglet folks (http://www.pyglet.org/) who wrote code that was much easier to
// understand than that! --jlnr

#include <Gosu/Window.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Utility.hpp>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "X11vroot.h"

#include <X11/extensions/Xinerama.h>

using namespace std::tr1::placeholders;

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }

    void screenMetrics(int *x_org, int *y_org, int *width, int *height){
        // Open the X Display; passing NULL returns the default display.
        Display* display = XOpenDisplay(NULL);
        
        // Raw screen information from the X server.
        Screen* screen = XScreenOfDisplay(display, DefaultScreen(display));
        
        // Xinerama screen information; if available, this info is more accurate.
        // This is especially important for multi-monitor configurations.
        int screen_count = 0;
        XineramaScreenInfo *screen_info = XineramaQueryScreens(display, &screen_count);
    
        // If screen_info is not NULL, we got preferred measurements from Xinerama,
        // otherwise we use the measurements from the X server.
        if(screen_info != NULL){
            // screen_info is an array of length screen_count
            // Index zero should hold the "default" or "primary"
            // screen as configured by the user.
            *x_org = screen_info[0].x_org;
            *y_org = screen_info[0].y_org;
            *width = screen_info[0].width;
            *height = screen_info[0].height;
        }else{
            // screen is a reference to the default X Server screen
            // Since we know Xinerama isn't running, this screen
            // should correspond to exactly one physical display.
            *x_org = 0;
            *y_org = 0;
            *width = screen->width;
            *height = screen->height;
        }
    
        // Release the Xinerama screen info, if we have it.
        if(screen_info != NULL){
            XFree(screen_info);
        }
        
        // Release the connection to the X Display
        XCloseDisplay(display);
        
        return;
    }

    unsigned screenWidth()
    {
        int x_org, y_org, width, height;
        screenMetrics(&x_org, &y_org, &width, &height);
        return width;
    }
    
    unsigned screenHeight()
    {
        int x_org, y_org, width, height;
        screenMetrics(&x_org, &y_org, &width, &height);
        return height;
    }
}

struct Gosu::Window::Impl
{
    GOSU_UNIQUE_PTR<Graphics> graphics;
    GOSU_UNIQUE_PTR<Input> input;

    ::Display* display;
    
    bool mapped, showing, active;
    
    Cursor emptyCursor;
    bool showingCursor;
    
    ::GLXContext context;
    ::Window window;
    ::XVisualInfo* visual;
    
    // Last set title
    std::wstring title;
    // Last known position
    int x, y;
    // Last known size
    int width, height;

    double updateInterval;
    bool fullscreen;

    Impl(unsigned width, unsigned height, unsigned fullscreen, double updateInterval)
    :   mapped(false), showing(false), active(true),
        x(0), y(0), width(width), height(height),
        updateInterval(updateInterval), fullscreen(fullscreen)
    {
        
    }
    
    void executeAndWait(std::tr1::function<void(Display*, ::Window)> function, int forMessage)
    {
        XSelectInput(display, window, StructureNotifyMask);
        function(display, window);
        while (true)
        {
            ::XEvent event;
            XNextEvent(display, &event);
            if (event.type == forMessage)
                break;
        }
        XSelectInput(display, window, 0x1ffffff & ~PointerMotionHintMask & ~ResizeRedirectMask);
    }

    void doTick(Window* window)
    {
        for (int i = XPending(display); i > 0; --i)
        {
            XEvent event;
            XNextEvent(display, &event);

            // Override redirect fix (thanks go to the Pyglet folks again):
            if (event.type == ButtonPress && fullscreen && !active)
                XSetInputFocus(display, this->window, RevertToParent, CurrentTime);
            
            if (!window->input().feedXEvent(event))
            {
                if (event.type == ConfigureNotify)
                {
                    // Only boring stuff to do? Let's do something random:
                    glXMakeCurrent(display, this->window, context);
                }
                else if (event.type == ClientMessage)
                {
                    if (static_cast<unsigned>(event.xclient.data.l[0]) ==
                            XInternAtom(display, "WM_DELETE_WINDOW", false))
                        window->close();
                }
                else if (event.type == FocusIn)
                    active = true;
                else if (event.type == FocusOut)
                    active = false;
            }
            if (event.type == Expose && event.xexpose.count == 0 &&
                        window->graphics().begin(Color::BLACK)) {
                FPS::registerFrame();
                window->draw();
                window->graphics().end();
                glXSwapBuffers(display, this->window);
            }
        }
        
        if (showingCursor && !window->needsCursor())
        {
            XDefineCursor(display, this->window, emptyCursor);
            showingCursor = false;
        }
        else if (!showingCursor && window->needsCursor())
        {
            XUndefineCursor(display, this->window);
            showingCursor = true;
        }
        
        Song::update();
        window->input().update();
        window->update();

        if (window->needsRedraw() && window->graphics().begin(Color::BLACK))
        {
            FPS::registerFrame();
            window->draw();
            window->graphics().end();
            glXSwapBuffers(display, this->window);
        }
    }
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
        double updateInterval)
:   pimpl(new Impl(width, height, fullscreen, updateInterval))
{
    pimpl->display = XOpenDisplay(NULL);
    if (!pimpl->display)
        throw std::runtime_error("Cannot find display");
    
    ::Window root = DefaultRootWindow(pimpl->display);

    // Setup GLX visual
    static int glxAttributes[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_DEPTH_SIZE, 1,
        None
    };
    pimpl->visual = glXChooseVisual(pimpl->display, DefaultScreen(pimpl->display), glxAttributes);

    // Create GLX context    
    pimpl->context = glXCreateContext(pimpl->display, pimpl->visual, 0, GL_TRUE);

    // Set up window attributes (& mask)
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = XCreateColormap(pimpl->display, root, pimpl->visual->visual, AllocNone);
    windowAttributes.bit_gravity = NorthWestGravity;
    windowAttributes.background_pixel = 0;
    unsigned mask = CWColormap | CWBitGravity | CWBackPixel;

    // Create window
    pimpl->window = XCreateWindow(pimpl->display, root, 0, 0, width, height, 0,
        pimpl->visual->depth, InputOutput, pimpl->visual->visual,
        mask, &windowAttributes);    

    // Request a close button for the window
    Atom atoms[] = { XInternAtom(pimpl->display, "WM_DELETE_WINDOW", false) };
    XSetWMProtocols(pimpl->display, pimpl->window, atoms, 1);


    // Get reference to X Screen
    Screen* screen = XScreenOfDisplay(pimpl->display,
                     DefaultScreen(pimpl->display));

    if (fullscreen){
        // If we're going fullscreen, replace the window
        // position and size with our screen metrics.
        int screen_x_org, screen_y_org, screen_width, screen_height;
        Gosu::screenMetrics(&screen_x_org, &screen_y_org, &screen_width, &screen_height);
        pimpl->width = screen_width;
        pimpl->height = screen_height;
        pimpl->x = screen_x_org;
        pimpl->y = screen_y_org;

        // Override Redirect (JohnColburn says: I don't actually know what this is for.)
        XSetWindowAttributes windowAttributes;
        windowAttributes.override_redirect = true;
        unsigned mask = CWOverrideRedirect;
        XChangeWindowAttributes(pimpl->display, pimpl->window, mask, &windowAttributes);
    }
    
    // Move and resize the window to its current position and size.
    XMoveResizeWindow(pimpl->display, pimpl->window, pimpl->x, pimpl->y, pimpl->width, pimpl->height);

    // Set window to be non-resizable
    XSizeHints *sizeHints = XAllocSizeHints();
    sizeHints->flags = PMinSize | PMaxSize;
    sizeHints->min_width = sizeHints->max_width = pimpl->width;
    sizeHints->min_height = sizeHints->max_height = pimpl->height;
    XSetWMNormalHints(pimpl->display, pimpl->window, sizeHints);
    XFree(sizeHints);

    // TODO: Window style (_MOTIF_WM_HINTS)?        

    XColor black, dummy;
    XAllocNamedColor(pimpl->display, screen->cmap, "black", &black, &dummy);    
    char emptyData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    Pixmap emptyBitmap =
        XCreateBitmapFromData(pimpl->display, pimpl->window, emptyData, 8, 8);
    pimpl->emptyCursor = XCreatePixmapCursor(pimpl->display, emptyBitmap,
        emptyBitmap, &black, &black, 0, 0);
    pimpl->showingCursor = true; // Empty cursor not yet installed

    // Must be current already so that Graphics' constructor can set up things
    glXMakeCurrent(pimpl->display, pimpl->window, pimpl->context);

    // Now set up major Gosu components
    pimpl->graphics.reset(new Graphics(pimpl->width, pimpl->height, fullscreen));
    pimpl->input.reset(new Input(pimpl->display, pimpl->window));    
    input().onButtonDown = std::tr1::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = std::tr1::bind(&Window::buttonUp, this, _1);
    
    // Fix coordinates for fullscreen screen-scaling
    if (fullscreen)
    {
        graphics().setResolution(width, height);
        input().setMouseFactors(1.0 * width / pimpl->width,
                  1.0 * height / pimpl->height);
    }
}

Gosu::Window::~Window()
{
    XFreeCursor(pimpl->display, pimpl->emptyCursor);
    XDestroyWindow(pimpl->display, pimpl->window);
    XSync(pimpl->display, false);
}

std::wstring Gosu::Window::caption() const
{
    return pimpl->title;
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    // TODO: Update to _NET_WM_NAME to support Unicode

    pimpl->title = caption;

    std::string tmpString(pimpl->title.begin(), pimpl->title.end());
    std::vector<char> title(pimpl->title.size() + 1);
    std::copy(tmpString.begin(), tmpString.end(), title.begin());
    title.back() = 0;

    XTextProperty titleprop;
    char* titlePtr = &title[0];
    XStringListToTextProperty(&titlePtr, 1, &titleprop);

    XSetWMName(pimpl->display, pimpl->window, &titleprop);
    XFree(titleprop.value);
    XSync(pimpl->display, false);
}

namespace GosusDarkSide
{
    // TODO: Find a way for this to fit into Gosu's design.
    // This can point to a function that wants to be called every
    // frame, e.g. rb_thread_schedule.
    typedef void (*HookOfHorror)();
    HookOfHorror oncePerTick = 0;
}

// TODO: Some exception safety

void Gosu::Window::show()
{
    // Map window
    pimpl->executeAndWait(XMapRaised, MapNotify);
    pimpl->mapped = true;
    
    // Make glx current
    glXMakeCurrent(pimpl->display, pimpl->window, pimpl->context);
    
    if (pimpl->fullscreen){
        XSetInputFocus(pimpl->display, pimpl->window, RevertToParent, CurrentTime);
        XGrabPointer(pimpl->display, pimpl->window, true, 0, GrabModeAsync, GrabModeAsync, pimpl->window, None, CurrentTime);
    }
    
    setCaption(pimpl->title);

    unsigned startTime, endTime;

    pimpl->showing = true;
    while (pimpl->showing)
    {
        startTime = milliseconds();
        pimpl->doTick(this);
        if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
        endTime = milliseconds();

        if (startTime <= endTime && (endTime - startTime) < pimpl->updateInterval)
            sleep(pimpl->updateInterval - (endTime - startTime));
    }

    glXMakeCurrent(pimpl->display, 0, 0);
    pimpl->executeAndWait(XUnmapWindow, UnmapNotify);
    pimpl->mapped = false;
}

void Gosu::Window::close()
{
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
            std::printf("glXMakeCurrent failed\n");
    }

    void releaseContext(Display* dpy, GLXContext context) {
        glXDestroyContext(dpy, context);
    }
}

Gosu::Window::SharedContext Gosu::Window::createSharedContext() {
    const char* displayName = DisplayString( pimpl->display );
    Display* dpy2 = XOpenDisplay( displayName );
    if (!dpy2)
        throw std::runtime_error("Could not duplicate X display");
    
    GLXContext ctx = glXCreateContext(dpy2, pimpl->visual, pimpl->context, True);
    if (!ctx)
        throw std::runtime_error("Could not create shared GLX context");
    
    return SharedContext(
        new std::tr1::function<void()>(std::tr1::bind(makeCurrentContext, dpy2, pimpl->window, ctx)),
            std::tr1::bind(releaseContext, dpy2, ctx));
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

