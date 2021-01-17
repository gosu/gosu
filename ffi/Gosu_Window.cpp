#include "Gosu_FFI_internal.h"

// Lifecycle

GOSU_FFI_API Gosu_Window* Gosu_Window_create(int width, int height, unsigned window_flags,
                                             double update_interval)
{
    return new Gosu_Window{width, height, window_flags, update_interval};
};

GOSU_FFI_API void Gosu_Window_destroy(Gosu_Window* window)
{
    delete window;
}

// Callbacks

GOSU_FFI_API void Gosu_Window_set_update(Gosu_Window* window, void function(void*), void* data)
{
    window->update_callback = [=] { function(data); };
}

GOSU_FFI_API void Gosu_Window_set_draw(Gosu_Window* window, void function(void*), void* data)
{
    window->draw_callback = [=] { function(data); };
}

GOSU_FFI_API void Gosu_Window_set_button_down(Gosu_Window* window, void function(void*, unsigned),
                                              void* data)
{
    window->button_down_callback = [=](unsigned btn) { function(data, btn); };
}

GOSU_FFI_API void Gosu_Window_set_button_up(Gosu_Window* window, void function(void*, unsigned),
                                            void* data)
{
    window->button_up_callback = [=](unsigned btn) { function(data, btn); };
}

GOSU_FFI_API void Gosu_Window_set_gamepad_connected(Gosu_Window* window, void function(void*, int),
                                                    void* data)
{
    window->gamepad_connected_callback = [=](int id) { function(data, id); };
}

GOSU_FFI_API void Gosu_Window_set_gamepad_disconnected(Gosu_Window* window,
                                                       void function(void*, int), void* data)
{
    window->gamepad_disconnected_callback = [=](int id) { function(data, id); };
}

GOSU_FFI_API void Gosu_Window_set_drop(Gosu_Window* window, void function(void*, const char*),
                                       void* data)
{
    window->drop_callback = [=](const char* filename) { function(data, filename); };
}

GOSU_FFI_API void Gosu_Window_set_needs_redraw(Gosu_Window* window, bool function(void*),
                                               void* data)
{
    window->needs_redraw_callback = [=] { return function(data); };
}

GOSU_FFI_API void Gosu_Window_set_needs_cursor(Gosu_Window* window, bool function(void*),
                                               void* data)
{
    window->needs_cursor_callback = [=] { return function(data); };
}

GOSU_FFI_API void Gosu_Window_set_close(Gosu_Window* window, void function(void*), void* data)
{
    window->close_callback = [=] { function(data); };
}

GOSU_FFI_API void Gosu_Window_default_button_down(Gosu_Window* window, unsigned id)
{
    window->Gosu::Window::button_down(static_cast<Gosu::Button>(id));
}

// Properties

GOSU_FFI_API int Gosu_Window_width(Gosu_Window* window)
{
    return window->width();
}

GOSU_FFI_API void Gosu_Window_set_width(Gosu_Window* window, int width)
{
    window->resize(width, window->height(), window->fullscreen());
}

GOSU_FFI_API int Gosu_Window_height(Gosu_Window* window)
{
    return window->height();
}

GOSU_FFI_API void Gosu_Window_set_height(Gosu_Window* window, int height)
{
    window->resize(window->width(), height, window->fullscreen());
}

GOSU_FFI_API bool Gosu_Window_is_fullscreen(Gosu_Window* window)
{
    return window->fullscreen();
}

GOSU_FFI_API void Gosu_Window_set_fullscreen(Gosu_Window* window, bool fullscreen)
{
    window->resize(window->width(), window->height(), fullscreen);
}

GOSU_FFI_API void Gosu_Window_resize(Gosu_Window* window, int width, int height, bool fullscreen)
{
    window->resize(width, height, fullscreen);
}

GOSU_FFI_API bool Gosu_Window_is_resizable(Gosu_Window* window)
{
    return window->resizable();
}

GOSU_FFI_API void Gosu_Window_set_resizable(Gosu_Window* window, bool resizable)
{
    window->set_resizable(resizable);
}

GOSU_FFI_API bool Gosu_Window_is_borderless(Gosu_Window* window)
{
    return window->borderless();
}

GOSU_FFI_API void Gosu_Window_set_borderless(Gosu_Window* window, bool borderless)
{
    window->set_borderless(borderless);
}

GOSU_FFI_API double Gosu_Window_update_interval(Gosu_Window* window)
{
    return window->update_interval();
}

GOSU_FFI_API void Gosu_Window_set_update_interval(Gosu_Window* window, double update_interval)
{
    window->set_update_interval(update_interval);
}

GOSU_FFI_API const char* Gosu_Window_caption(Gosu_Window* window)
{
    static thread_local std::string caption;
    caption = window->caption();
    return caption.c_str();
}

GOSU_FFI_API void Gosu_Window_set_caption(Gosu_Window* window, const char* caption)
{
    window->set_caption(caption);
}

// Input Properties

GOSU_FFI_API Gosu_TextInput* Gosu_Window_text_input(Gosu_Window* window)
{
    return dynamic_cast<Gosu_TextInput*>(window->input().text_input());
}

GOSU_FFI_API void Gosu_Window_set_text_input(Gosu_Window* window, Gosu_TextInput* text_input)
{
    window->input().set_text_input(text_input);
}

GOSU_FFI_API double Gosu_Window_mouse_x(Gosu_Window* window)
{
    return window->input().mouse_x();
}

GOSU_FFI_API void Gosu_Window_set_mouse_x(Gosu_Window* window, double x)
{
    window->input().set_mouse_position(x, window->input().mouse_x());
}

GOSU_FFI_API double Gosu_Window_mouse_y(Gosu_Window* window)
{
    return window->input().mouse_y();
}

GOSU_FFI_API void Gosu_Window_set_mouse_y(Gosu_Window* window, double y)
{
    window->input().set_mouse_position(window->input().mouse_x(), y);
}

// Main Loop

GOSU_FFI_API void Gosu_Window_show(Gosu_Window* window)
{
    window->show();
}

GOSU_FFI_API bool Gosu_Window_tick(Gosu_Window* window)
{
    return window->tick();
}

GOSU_FFI_API void Gosu_Window_close_immediately(Gosu_Window* window)
{
    window->Gosu::Window::close();
}
