#include <Gosu/Platform.hpp>
#if !defined(GOSU_IS_IPHONE)

#include <Gosu/Gosu.hpp>
#include <SDL.h>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

namespace Gosu
{
    namespace FPS
    {
        void register_frame();
    }

    static void throw_sdl_error(const std::string& operation)
    {
        const char* error = SDL_GetError();
        throw std::runtime_error(operation + ": " + (error ? error : "(unknown error)"));
    }

    static void cleanup();

    static SDL_Window* shared_window()
    {
        static SDL_Window* window = nullptr;
        if (window == nullptr) {
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                throw_sdl_error("Could not initialize SDL Video");
            }

            std::atexit(cleanup);

            Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
            
        #if SDL_VERSION_ATLEAST(2, 0, 1)
            flags |= SDL_WINDOW_ALLOW_HIGHDPI;
        #endif
            
            window =
                SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64, 64, flags);
            if (window == nullptr) {
                throw_sdl_error("Could not create window");
            }
        }
        return window;
    }

    static SDL_GLContext shared_gl_context()
    {
        static SDL_GLContext context = nullptr;
        if (context == nullptr) {
        #ifdef GOSU_IS_OPENGLES
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        #endif
            
            context = SDL_GL_CreateContext(shared_window());
            
            if (context == nullptr) {
                throw_sdl_error("Could not create OpenGL context");
            }
        }
        return context;
    }
    
    void ensure_current_context()
    {
        SDL_GL_MakeCurrent(shared_window(), shared_gl_context());
    }

    static void cleanup()
    {
        SDL_GL_DeleteContext(shared_gl_context());
        SDL_DestroyWindow(shared_window());
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

struct Gosu::Window::Impl
{
    bool fullscreen;
    double update_interval;
    
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen, double update_interval)
: pimpl(new Impl)
{
    // Even in fullscreen mode, temporarily show the window in windowed mode to centre it.
    // This ensures that the window will be centred correctly when exiting fullscreen mode.
    // Fixes https://github.com/gosu/gosu/issues/369
    resize(width, height, false);
    SDL_SetWindowPosition(shared_window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // This will implicitly create graphics() and input(), and make the OpenGL context current.
    resize(width, height, fullscreen);
    
    SDL_GL_SetSwapInterval(1);

    pimpl->update_interval = update_interval;
    
    input().on_button_down = [this](Button button) { button_down(button); };
    input().on_button_up = [this](Button button) { button_up(button); };
}

Gosu::Window::~Window()
{
    SDL_HideWindow(shared_window());
}

unsigned Gosu::Window::width() const
{
    return graphics().width();
}

unsigned Gosu::Window::height() const
{
    return graphics().height();
}

bool Gosu::Window::fullscreen() const
{
    return pimpl->fullscreen;
}

void Gosu::Window::resize(unsigned width, unsigned height, bool fullscreen)
{
    pimpl->fullscreen = fullscreen;
    
    int actual_width = width;
    int actual_height = height;
    double scale_factor = 1.0;
    double black_bar_width = 0;
    double black_bar_height = 0;
    
    if (fullscreen) {
        actual_width = Gosu::screen_width();
        actual_height = Gosu::screen_height();

        double scale_x = 1.0 * actual_width / width;
        double scale_y = 1.0 * actual_height / height;
        scale_factor = std::min(scale_x, scale_y);

        if (scale_x < scale_y) {
            black_bar_height = (actual_height / scale_x - height) / 2;
        }
        else if (scale_y < scale_x) {
            black_bar_width = (actual_width / scale_y - width) / 2;
        }
    }
    else {
        double max_width = Gosu::available_width();
        double max_height = Gosu::available_height();
        
        if (width > max_width || height > max_height) {
            scale_factor = std::min(max_width / width, max_height / height);
            actual_width = width * scale_factor;
            actual_height = height * scale_factor;
        }
    }
    
    SDL_SetWindowFullscreen(shared_window(), fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SDL_SetWindowSize(shared_window(), actual_width, actual_height);
    
#if SDL_VERSION_ATLEAST(2, 0, 1)
    SDL_GL_GetDrawableSize(shared_window(), &actual_width, &actual_height);
#endif
    
    ensure_current_context();
    
    if (pimpl->graphics.get() == nullptr) {
        pimpl->graphics.reset(new Graphics(actual_width, actual_height));
    }
    else {
        pimpl->graphics->set_physical_resolution(actual_width, actual_height);
    }
    pimpl->graphics->set_resolution(width, height, black_bar_width, black_bar_height);
    
    if (pimpl->input.get() == nullptr) {
        pimpl->input.reset(new Input(shared_window()));
    }
    pimpl->input->set_mouse_factors(1 / scale_factor, 1 / scale_factor,
                                    black_bar_width, black_bar_height);
}

double Gosu::Window::update_interval() const
{
    return pimpl->update_interval;
}

void Gosu::Window::set_update_interval(double update_interval)
{
    pimpl->update_interval = update_interval;
}

std::string Gosu::Window::caption() const
{
    const char* title = SDL_GetWindowTitle(shared_window());
    return title ? title : "";
}

void Gosu::Window::set_caption(const std::string& caption)
{
    SDL_SetWindowTitle(shared_window(), caption.c_str());
}

void Gosu::Window::show()
{
    unsigned long time_before_tick = milliseconds();
    
    while (tick()) {
        // Sleep to keep this loop from eating 100% CPU.
        unsigned long tick_time = milliseconds() - time_before_tick;
        if (tick_time < update_interval()) {
            sleep(update_interval() - tick_time);
        }
        
        time_before_tick = milliseconds();
    }
}

bool Gosu::Window::tick()
{
    if (SDL_GetWindowFlags(shared_window()) & SDL_WINDOW_HIDDEN) {
        SDL_ShowWindow(shared_window());

        // SDL_GL_GetDrawableSize returns different values before and after showing the window.
        // -> When first showing the window, update the physical size of Graphics (=glViewport).
        // Fixes https://github.com/gosu/gosu/issues/318
        int width, height;
        SDL_GL_GetDrawableSize(shared_window(), &width, &height);
        graphics().set_physical_resolution(width, height);
    }
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            SDL_HideWindow(shared_window());
            return false;
        }
        else {
            input().feed_sdl_event(&e);
        }
    }
    
    Song::update();
    
    input().update();
    
    update();
    
    SDL_ShowCursor(needs_cursor());
    
    if (needs_redraw()) {
        ensure_current_context();
        if (graphics().begin()) {
            draw();
            graphics().end();
            FPS::register_frame();
        }
        
        SDL_GL_SwapWindow(shared_window());
    }
    
    return true;
}

void Gosu::Window::close()
{
    SDL_Event e;
    e.type = SDL_QUIT;
    SDL_PushEvent(&e);
}

void Gosu::Window::button_down(Button button)
{
    bool toggle_fullscreen;

    // Default shortcuts for toggling fullscreen mode, see: https://github.com/gosu/gosu/issues/361
    
#ifdef GOSU_IS_MAC
    // cmd+F and cmd+ctrl+F are both common shortcuts for toggling fullscreen mode on macOS.
    toggle_fullscreen = button == KB_F &&
        (Input::down(KB_LEFT_META) || Input::down(KB_RIGHT_META)) &&
        !Input::down(KB_LEFT_SHIFT) && !Input::down(KB_RIGHT_SHIFT) &&
        !Input::down(KB_LEFT_ALT) && !Input::down(KB_RIGHT_ALT);
#else
    // alt+enter and alt+return toggle fullscreen mode on all other platforms.
    toggle_fullscreen = (button == KB_RETURN || button == KB_ENTER) &&
        (Input::down(KB_LEFT_ALT) || Input::down(KB_RIGHT_ALT)) &&
        !Input::down(KB_LEFT_CONTROL) && !Input::down(KB_RIGHT_CONTROL) &&
        !Input::down(KB_LEFT_META) && !Input::down(KB_RIGHT_META) &&
        !Input::down(KB_LEFT_SHIFT) && !Input::down(KB_RIGHT_SHIFT);
#endif

    if (toggle_fullscreen) {
        resize(width(), height(), !fullscreen());
    }
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

#endif
