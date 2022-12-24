#include <Gosu/Platform.hpp>
#if !defined(GOSU_IS_IPHONE)

#if defined(GOSU_IS_WIN)
#define NOMINMAX
#include <windows.h>
#endif

#include <Gosu/Gosu.hpp>
#include "GraphicsImpl.hpp"
#include <SDL.h>
#include <algorithm>
#include <stdexcept>

namespace Gosu
{
    namespace FPS
    {
        void register_frame();
    }

    [[noreturn]] static void throw_sdl_error(const std::string& operation)
    {
        const char* error = SDL_GetError();
        throw std::runtime_error{operation + ": " + (error ? error : "(unknown error)")};
    }

    SDL_Window* shared_window()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
        static SDL_Window* window = nullptr;
        if (window == nullptr) {
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                throw_sdl_error("Could not initialize SDL Video");
            }

            Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI;

            window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64, 64,
                                      flags);
            if (window == nullptr) {
                throw_sdl_error("Could not create window");
            }
            SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
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

    void ensure_current_context() //
    {
        SDL_GL_MakeCurrent(shared_window(), shared_gl_context());
    }
}

struct Gosu::Window::Impl : private Gosu::Noncopyable
{
    bool fullscreen = false;
    double update_interval = 0;
    bool resizable = false;
    bool resizing = false;

    // A single `bool open` is not good enough to support the tick() method: When close() is called
    // from outside the window's call graph, the next call to tick() must return false (transition
    // from CLOSING to CLOSED), but the call after that must show the window again (transition from
    // CLOSED to OPEN).
    enum
    {
        CLOSED,
        OPEN,
        CLOSING
    } state = CLOSED;

    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;
};

Gosu::Window::Window(int width, int height, unsigned window_flags, double update_interval)
: m_impl(new Impl)
{
    set_borderless(window_flags & WF_BORDERLESS);
    set_resizable(window_flags & WF_RESIZABLE);

    // Even in fullscreen mode, temporarily show the window in windowed mode to center it.
    // This ensures that the window will be centered correctly when exiting fullscreen mode.
    // Fixes https://github.com/gosu/gosu/issues/369
    // (This will implicitly create graphics() and input(), and make the OpenGL context current.)
    resize(width, height, false);
    SDL_SetWindowPosition(shared_window(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Really enable fullscreen if desired.
    resize(width, height, (window_flags & WF_FULLSCREEN));

    SDL_GL_SetSwapInterval(1);

    m_impl->update_interval = update_interval;

    input().on_button_down = [this](Button button) { button_down(button); };
    input().on_button_up = [this](Button button) { button_up(button); };
    input().on_gamepad_connected = [this](int index) { gamepad_connected(index); };
    input().on_gamepad_disconnected = [this](int index) { gamepad_disconnected(index); };
}

Gosu::Window::~Window()
{
    SDL_HideWindow(shared_window());
}

int Gosu::Window::width() const
{
    return graphics().width();
}

int Gosu::Window::height() const
{
    return graphics().height();
}

bool Gosu::Window::fullscreen() const
{
    return m_impl->fullscreen;
}

void Gosu::Window::resize(int width, int height, bool fullscreen)
{
    m_impl->fullscreen = fullscreen;

    int actual_width = width;
    int actual_height = height;
    double scale_factor = 1.0;
    double black_bar_width = 0;
    double black_bar_height = 0;

    if (fullscreen) {
        actual_width = Gosu::screen_width(this);
        actual_height = Gosu::screen_height(this);

        if (resizable()) {
            // Resizable fullscreen windows stubbornly follow the desktop resolution.
            width = actual_width;
            height = actual_height;
        }
        else {
            // Scale the window to fill the desktop resolution.
            double scale_x = 1.0 * actual_width / width;
            double scale_y = 1.0 * actual_height / height;
            scale_factor = std::min(scale_x, scale_y);
            // Add black bars to preserve the aspect ratio, if necessary.
            if (scale_x < scale_y) {
                black_bar_height = (actual_height / scale_x - height) / 2;
            }
            else if (scale_y < scale_x) {
                black_bar_width = (actual_width / scale_y - width) / 2;
            }
        }
    }
    else {
        int max_width = Gosu::available_width(this);
        int max_height = Gosu::available_height(this);

        if (resizable()) {
            // If the window is resizable, limit its size, without preserving the aspect ratio.
            width = actual_width = std::min(width, max_width);
            height = actual_height = std::min(height, max_height);
        }
        else if (width > max_width || height > max_height) {
            // If the window cannot fit on the screen, shrink its contents.
            scale_factor = std::min(1.0 * max_width / width, 1.0 * max_height / height);
            actual_width = static_cast<int>(width * scale_factor);
            actual_height = static_cast<int>(height * scale_factor);
        }
    }

    SDL_SetWindowFullscreen(shared_window(), fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if (!m_impl->resizing) {
        SDL_SetWindowSize(shared_window(), actual_width, actual_height);
    }

    SDL_GL_GetDrawableSize(shared_window(), &actual_width, &actual_height);

    ensure_current_context();

    if (!m_impl->graphics) {
        m_impl->graphics.reset(new Graphics(actual_width, actual_height));
    }
    else {
        m_impl->graphics->set_physical_resolution(actual_width, actual_height);
    }
    m_impl->graphics->set_resolution(width, height, black_bar_width, black_bar_height);

    if (!m_impl->input) {
        m_impl->input.reset(new Input(shared_window()));
    }
    m_impl->input->set_mouse_factors(1 / scale_factor, 1 / scale_factor, black_bar_width,
                                     black_bar_height);
}

bool Gosu::Window::resizable() const
{
    return m_impl->resizable;
}

void Gosu::Window::set_resizable(bool resizable)
{
    m_impl->resizable = resizable;
    SDL_SetWindowResizable(shared_window(), resizable ? SDL_TRUE : SDL_FALSE);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool Gosu::Window::borderless() const
{
    return SDL_GetWindowFlags(shared_window()) & SDL_WINDOW_BORDERLESS;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Gosu::Window::set_borderless(bool borderless)
{
    SDL_SetWindowBordered(shared_window(), borderless ? SDL_FALSE : SDL_TRUE);
}

double Gosu::Window::update_interval() const
{
    return m_impl->update_interval;
}

void Gosu::Window::set_update_interval(double update_interval)
{
    m_impl->update_interval = update_interval;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string Gosu::Window::caption() const
{
    const char* title = SDL_GetWindowTitle(shared_window());
    return title ? title : "";
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Gosu::Window::set_caption(const std::string& caption)
{
    SDL_SetWindowTitle(shared_window(), caption.c_str());
}

void Gosu::Window::show()
{
    unsigned long time_before_tick = milliseconds();

#ifdef GOSU_IS_WIN
    // Try to convince Windows to only run this thread on the first core, to avoid timing glitches.
    // (If we ever run into a situation where the first core is not available, we should start to
    // use GetProcessAffinityMask to retrieve the allowed cores as a bitmask.)
    DWORD_PTR previous_affinity = SetThreadAffinityMask(GetCurrentThread(), 1);
#endif

    try {
        while (tick()) {
            // Sleep to keep this loop from eating 100% CPU.
            unsigned long tick_time = milliseconds() - time_before_tick;
            long sleep_time = static_cast<long>(update_interval() - static_cast<double>(tick_time));
            if (sleep_time >= 1) {
                // Use Gosu::sleep instead of std::this_thread::sleep_for because Win32 Sleep()
                // results in better behavior here, sleep_for causes FPS to drop from 60 to <50.
                // (This is also the reason why Gosu::sleep still exists.)
                Gosu::sleep(sleep_time);
            }

            time_before_tick = milliseconds();
        }
    } catch (...) {
#ifdef GOSU_IS_WIN
        if (previous_affinity) SetThreadAffinityMask(GetCurrentThread(), previous_affinity);
#endif
        throw;
    }

#ifdef GOSU_IS_WIN
    if (previous_affinity) SetThreadAffinityMask(GetCurrentThread(), previous_affinity);
#endif

    m_impl->state = Impl::CLOSED;
}

bool Gosu::Window::tick()
{
    if (m_impl->state == Impl::CLOSING) {
        m_impl->state = Impl::CLOSED;
        return false;
    }

    if (m_impl->state == Impl::CLOSED) {
        SDL_ShowWindow(shared_window());
        m_impl->state = Impl::OPEN;

        // SDL_GL_GetDrawableSize returns different values before and after showing the window.
        // -> When first showing the window, update the physical size of Graphics (=glViewport).
        // Fixes https://github.com/gosu/gosu/issues/318
        int width, height;
        SDL_GL_GetDrawableSize(shared_window(), &width, &height);
        graphics().set_physical_resolution(width, height);
    }

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_WINDOWEVENT: {
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        if (m_impl->resizable &&
                            (width() != e.window.data1 || height() != e.window.data2)) {
                            m_impl->resizing = true;
                            resize(e.window.data1, e.window.data2, fullscreen());
                            m_impl->resizing = false;
                        }
                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_GAINED: {
                        gain_focus();
                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_LOST: {
                        lose_focus();
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case SDL_QUIT: {
                close();
                break;
            }
            case SDL_DROPFILE: {
                std::shared_ptr<char> dropped_file{e.drop.file, SDL_free};
                if (dropped_file == nullptr) break;
                drop(dropped_file.get());
                break;
            }
            default: {
                input().feed_sdl_event(&e);
                break;
            }
        }
    }

    Song::update();

    input().update();

    update();

    SDL_ShowCursor(needs_cursor());

    if (needs_redraw()) {
        ensure_current_context();
        graphics().frame([&] {
            draw();
            FPS::register_frame();
        });

        SDL_GL_SwapWindow(shared_window());
    }

    if (m_impl->state == Impl::CLOSING) {
        m_impl->state = Impl::CLOSED;
    }

    return m_impl->state == Impl::OPEN;
}

void Gosu::Window::close()
{
    m_impl->state = Impl::CLOSING;
    SDL_HideWindow(shared_window());
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
    // Alt+Enter and Alt+Return toggle fullscreen mode on all other platforms.
    toggle_fullscreen = (button == KB_RETURN || button == KB_ENTER) &&
                        (Input::down(KB_LEFT_ALT) || Input::down(KB_RIGHT_ALT)) &&
                        !Input::down(KB_LEFT_CONTROL) && !Input::down(KB_RIGHT_CONTROL) &&
                        !Input::down(KB_LEFT_META) && !Input::down(KB_RIGHT_META) &&
                        !Input::down(KB_LEFT_SHIFT) && !Input::down(KB_RIGHT_SHIFT);
#endif
    // F11 is supported as a shortcut for fullscreen mode on all platforms.
    if (!toggle_fullscreen && button == KB_F11 && !Input::down(KB_LEFT_ALT) &&
        !Input::down(KB_RIGHT_ALT) && !Input::down(KB_LEFT_CONTROL) &&
        !Input::down(KB_RIGHT_CONTROL) && !Input::down(KB_LEFT_META) &&
        !Input::down(KB_RIGHT_META) && !Input::down(KB_LEFT_SHIFT) &&
        !Input::down(KB_RIGHT_SHIFT)) {
        toggle_fullscreen = true;
    }

    if (toggle_fullscreen) {
        resize(width(), height(), !fullscreen());
    }
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *m_impl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *m_impl->graphics;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *m_impl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *m_impl->input;
}

#endif
