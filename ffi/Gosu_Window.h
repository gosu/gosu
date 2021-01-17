#pragma once

#include "Gosu_FFI.h"
#include "Gosu_TextInput.h"
#include <stdbool.h>

typedef struct Gosu_Window Gosu_Window;

// Lifecycle

GOSU_FFI_API Gosu_Window* Gosu_Window_create(int width, int height, unsigned window_flags,
                                             double update_interval);
GOSU_FFI_API void Gosu_Window_destroy(Gosu_Window* window);

// Callbacks

GOSU_FFI_API void Gosu_Window_set_update(Gosu_Window* window, void function(void* data),
                                         void* data);
GOSU_FFI_API void Gosu_Window_set_draw(Gosu_Window* window, void function(void* data), void* data);
GOSU_FFI_API void Gosu_Window_set_button_down(Gosu_Window* window,
                                              void function(void* data, unsigned id), void* data);
GOSU_FFI_API void Gosu_Window_set_button_up(Gosu_Window* window,
                                            void function(void* data, unsigned id), void* data);
GOSU_FFI_API void Gosu_Window_set_gamepad_connected(Gosu_Window* window,
                                                    void function(void* data, int index),
                                                    void* data);
GOSU_FFI_API void Gosu_Window_set_gamepad_disconnected(Gosu_Window* window,
                                                       void function(void* data, int index),
                                                       void* data);
GOSU_FFI_API void Gosu_Window_set_drop(Gosu_Window* window,
                                       void function(void* data, const char* filename), void* data);
GOSU_FFI_API void Gosu_Window_set_needs_redraw(Gosu_Window* window, bool function(void* data),
                                               void* data);
GOSU_FFI_API void Gosu_Window_set_needs_cursor(Gosu_Window* window, bool function(void* data),
                                               void* data);
GOSU_FFI_API void Gosu_Window_set_close(Gosu_Window* window, void function(void* data), void* data);

GOSU_FFI_API void Gosu_Window_default_button_down(Gosu_Window* window, unsigned id);

// Properties

GOSU_FFI_API int Gosu_Window_width(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_width(Gosu_Window* window, int width);
GOSU_FFI_API int Gosu_Window_height(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_height(Gosu_Window* window, int height);
GOSU_FFI_API bool Gosu_Window_is_fullscreen(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_fullscreen(Gosu_Window* window, bool fullscreen);
GOSU_FFI_API void Gosu_Window_resize(Gosu_Window* window, int width, int height, bool fullscreen);

GOSU_FFI_API bool Gosu_Window_is_resizable(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_resizable(Gosu_Window* window, bool resizable);
GOSU_FFI_API bool Gosu_Window_is_borderless(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_borderless(Gosu_Window* window, bool borderless);

GOSU_FFI_API double Gosu_Window_update_interval(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_update_interval(Gosu_Window* window, double update_interval);

GOSU_FFI_API const char* Gosu_Window_caption(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_caption(Gosu_Window* window, const char* caption);

// Input Properties

GOSU_FFI_API Gosu_TextInput* Gosu_Window_text_input(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_text_input(Gosu_Window* window, Gosu_TextInput* text_input);
GOSU_FFI_API double Gosu_Window_mouse_x(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_mouse_x(Gosu_Window* window, double width);
GOSU_FFI_API double Gosu_Window_mouse_y(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_set_mouse_y(Gosu_Window* window, double height);

// Main Loop

GOSU_FFI_API void Gosu_Window_show(Gosu_Window* window);
GOSU_FFI_API bool Gosu_Window_tick(Gosu_Window* window);
GOSU_FFI_API void Gosu_Window_close_immediately(Gosu_Window* window);
