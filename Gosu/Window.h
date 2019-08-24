#pragma once

#ifdef __cplusplus
  namespace Gosu
  {
    class WindowForWrapper : public Gosu::Window
    {
    struct Callbacks
    {
      void (*update)();
      void (*draw)();
    } callbacks;

    public:
        WindowForWrapper(int width, int height, bool fullscreen, double update_interval, bool resizable);
        void set_draw(void function());
        void set_update(void function());
        void update() override;
        void draw() override;
    };
  }

  extern "C" {
#endif

typedef struct Gosu_Window Gosu_Window;

// Constructor
Gosu_Window* Gosu_Window_create(int width, int height, int fullscreen, double update_interval, int resizable);

// callbacks
void Gosu_Window_update(Gosu_Window* window, void function());
void Gosu_Window_draw(Gosu_Window* window, void function());
void Gosu_Window_button_down(Gosu_Window* window, void function());
void Gosu_Window_button_up(Gosu_Window* window, void function());
void Gosu_Window_drop(Gosu_Window* window, void function());
void Gosu_Window_needs_redraw(Gosu_Window* window, void function());
void Gosu_Window_needs_cursor(Gosu_Window* window, void function());

// Properties
int Gosu_Window_width(Gosu_Window* window);
int Gosu_Window_height(Gosu_Window* window);
int Gosu_Window_fullscreen(Gosu_Window* window);
double Gosu_Window_update_interval(Gosu_Window* window);
int Gosu_Window_resizable(Gosu_Window* window);
const char* Gosu_Window_caption(Gosu_Window* window);

int Gosu_Window_text_input(Gosu_Window* window); // TODO: implement Gosu_TextInput
double Gosu_Window_mouse_x(Gosu_Window* window);
double Gosu_Window_mouse_y(Gosu_Window* window);

// Setters
void Gosu_Window_set_text_input(Gosu_Window* window); // Gosu_TextInput* text_input
void Gosu_Window_set_caption(Gosu_Window* window, const char* caption);
void Gosu_Window_set_update_interval(Gosu_Window* window, double update_interval);
void Gosu_Window_set_mouse_x(Gosu_Window* window, int width);
void Gosu_Window_set_mouse_y(Gosu_Window* window, int height);

void Gosu_Window_resize(Gosu_Window* window, int width, int height, int fullscreen);

// Main Loop
void Gosu_Window_show(Gosu_Window* window);
int Gosu_Window_tick(Gosu_Window* window);

// Destructor
void Gosu_Window_destroy(Gosu_Window* window);


#ifdef __cplusplus
  }
#endif