#pragma once
#include <stdbool.h>
#include "TextInput.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Window Gosu_Window;

// Constructor
Gosu_Window* Gosu_Window_create(int width, int height, bool fullscreen, double update_interval, bool resizable);

// callbacks
void Gosu_Window_set_update(Gosu_Window *window, void function(void *data), void* data);
void Gosu_Window_set_draw(Gosu_Window *window, void function(void *data), void* data);
void Gosu_Window_set_button_down(Gosu_Window *window, void function(void *data, unsigned btn), void* data);
void Gosu_Window_set_button_up(Gosu_Window *window, void function(void *data, unsigned btn), void* data);
void Gosu_Window_set_gamepad_connected(Gosu_Window *window, void function(void *data, int id), void* data);
void Gosu_Window_set_gamepad_disconnected(Gosu_Window *window, void function(void *data, int id), void* data);
void Gosu_Window_set_drop(Gosu_Window *window, void function(void *data, const char *filename), void* data);
void Gosu_Window_set_needs_redraw(Gosu_Window *window, bool function(void *data), void* data);
void Gosu_Window_set_needs_cursor(Gosu_Window *window, bool function(void *data), void* data);
void Gosu_Window_set_close(Gosu_Window *window, void function(void *data), void* data);

void Gosu_Window_default_button_down(Gosu_Window* window, unsigned btn);

// Properties
int Gosu_Window_width(Gosu_Window* window);
int Gosu_Window_height(Gosu_Window* window);
bool Gosu_Window_is_fullscreen(Gosu_Window* window);
bool Gosu_Window_is_resizable(Gosu_Window* window);
double Gosu_Window_update_interval(Gosu_Window* window);
const char* Gosu_Window_caption(Gosu_Window* window);

Gosu_TextInput* Gosu_Window_text_input(Gosu_Window *window);
double Gosu_Window_mouse_x(Gosu_Window* window);
double Gosu_Window_mouse_y(Gosu_Window* window);
int Gosu_Window_is_button_down(Gosu_Window* window, unsigned btn);

// Setters
void Gosu_Window_set_text_input(Gosu_Window *window, Gosu_TextInput *text_input);
void Gosu_Window_set_caption(Gosu_Window* window, const char* caption);
void Gosu_Window_set_update_interval(Gosu_Window* window, double update_interval);
void Gosu_Window_set_mouse_x(Gosu_Window* window, double width);
void Gosu_Window_set_mouse_y(Gosu_Window* window, double height);
void Gosu_Window_set_width(Gosu_Window* window, int width);
void Gosu_Window_set_height(Gosu_Window* window, int height);

void Gosu_Window_resize(Gosu_Window* window, int width, int height, bool fullscreen);

// Main Loop
void Gosu_Window_show(Gosu_Window* window);
void Gosu_Window_close_immediately(Gosu_Window* window);
bool Gosu_Window_tick(Gosu_Window* window);

// Destructor
void Gosu_Window_destroy(Gosu_Window* window);


#ifdef __cplusplus
}
#endif
