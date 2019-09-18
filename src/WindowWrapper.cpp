#include <Gosu/Gosu.hpp>

namespace Gosu
{
  class WindowForWrapper : public Gosu::Window
  {
  public:
    WindowForWrapper(int width, int height, bool fullscreen, double update_interval, bool resizable);
    void set_draw(void function(void *data), void *data);
    void set_update(void function(void *data), void *data);
    void set_button_down(void function(void *data, unsigned btn), void *data);
    void set_button_up(void function(void *data, unsigned btn), void *data);
    void set_drop(void function(void *data, const char *filename), void *data);
    void set_needs_redraw(bool function(void *data), void *data);
    void set_needs_cursor(bool function(void *data), void *data);
    void set_close(void function(void *data), void *data);

    void update() override;
    void draw() override;
    void gosu_button_down(unsigned btn); // Enables fullscreen toggle
    void button_down(Gosu::Button btn) override;
    void button_up(Gosu::Button btn) override;
    void drop(const std::string& filename) override;
    bool needs_redraw() const override;
    bool needs_cursor() const override;
    void close() override;

    void close_immediately();

    // Callback function pointers
    // All explicitly set as nullptr to prevent segfault
    std::function<void (void)> update_callback;
    std::function<void (void)> draw_callback;
    std::function<void (unsigned btn)> button_down_callback;
    std::function<void (unsigned btn)> button_up_callback;
    std::function<void (const char* filename)> drop_callback;
    std::function<bool (void)> needs_redraw_callback;
    std::function<bool (void)> needs_cursor_callback;
    std::function<void (void)> close_callback;
  };
}

Gosu::WindowForWrapper::WindowForWrapper(int width, int height, bool fullscreen,
                                         double update_interval, bool resizable)
: Gosu::Window(width, height, fullscreen, update_interval, resizable) {
}

void Gosu::WindowForWrapper::set_update(void function(void *data), void *data)
{
  update_callback = [=](void) { function(data); };
}

void Gosu::WindowForWrapper::set_draw(void function(void *data), void *data)
{
  draw_callback = [=](void) { function(data); };
}

void Gosu::WindowForWrapper::set_button_down(void function(void *data, unsigned btn), void *data)
{
  button_down_callback = [=](unsigned btn) { function(data, btn); };
}

void Gosu::WindowForWrapper::set_button_up(void function(void *data, unsigned btn), void *data)
{
  button_up_callback = [=](unsigned btn) { function(data, btn); };
}

void Gosu::WindowForWrapper::set_drop(void function(void *data, const char *filename), void *data)
{
  drop_callback = [=](const char* filename){ function(data, filename); };
}

void Gosu::WindowForWrapper::set_needs_redraw(bool function(void *data), void *data)
{
  needs_redraw_callback = [=](void)->bool { return function(data); };
}

void Gosu::WindowForWrapper::set_needs_cursor(bool function(void *data), void *data)
{
  needs_cursor_callback = [=](void)->bool { return function(data); };
}

void Gosu::WindowForWrapper::set_close(void function(void *data), void *data)
{
  close_callback = [=](void) { function(data); };
}

void Gosu::WindowForWrapper::update()
{
  if (update_callback != nullptr) {
    update_callback();
  }
}

void Gosu::WindowForWrapper::draw()
{
  if (draw_callback != nullptr) {
    draw_callback();
  }
}

void Gosu::WindowForWrapper::gosu_button_down(unsigned btn)
{
  Gosu::Window::button_down(Gosu::ButtonName(btn));
}

void Gosu::WindowForWrapper::button_down(Gosu::Button btn)
{
  if (button_down_callback != nullptr) {
    button_down_callback(btn.id());
  }
}

void Gosu::WindowForWrapper::button_up(Gosu::Button btn)
{
  if (button_up_callback != nullptr) {
    button_up_callback(btn.id());
  }
}

void Gosu::WindowForWrapper::drop(const std::string& filename)
{
  if (drop_callback != nullptr) {
    drop_callback(filename.c_str());
  }
}

bool Gosu::WindowForWrapper::needs_redraw() const
{
  if (needs_redraw_callback != nullptr) {
    return needs_redraw_callback();
  }
  else {
    return true;
  }
}

bool Gosu::WindowForWrapper::needs_cursor() const
{
  if (needs_cursor_callback != nullptr) {
    return needs_cursor_callback();
  }
  else {
    return false;
  }
}

void Gosu::WindowForWrapper::close()
{
  if (close_callback != nullptr) {
    close_callback();
  }
  else {
    Gosu::Window::close();
  }
}

void Gosu::WindowForWrapper::close_immediately()
{
  Gosu::Window::close();
}

extern "C" {
  #include <Gosu/Window.h>
  #include <Gosu/TextInput.h>

  // Constructor
  Gosu_Window* Gosu_Window_create(int width, int height, bool fullscreen, double update_interval, bool resizable)
  {
    return reinterpret_cast<Gosu_Window*>( new Gosu::WindowForWrapper(width, height, fullscreen, update_interval, resizable) );
  };

  // Callbacks
  void Gosu_Window_set_draw(Gosu_Window *window, void function(void *data), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_draw(function, data);
  }

  void Gosu_Window_set_update(Gosu_Window *window, void function(void *data), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_update(function, data);
  }

  void Gosu_Window_set_button_down(Gosu_Window *window, void function(void *data, unsigned btn), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_button_down(function, data);
  }

  void Gosu_Window_gosu_button_down(Gosu_Window *window, unsigned btn)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->gosu_button_down(btn);
  }

  void Gosu_Window_set_button_up(Gosu_Window *window, void function(void *data, unsigned btn), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_button_up(function, data);
  }

  void Gosu_Window_set_drop(Gosu_Window *window, void function(void *data, const char *filename), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_drop(function, data);
  }

  void Gosu_Window_set_needs_redraw(Gosu_Window *window, bool function(void *data), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_needs_redraw(function, data);
  }

  void Gosu_Window_set_needs_cursor(Gosu_Window *window, bool function(void *data), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_needs_cursor(function, data);
  }

  void Gosu_Window_set_close(Gosu_Window *window, void function(void *data), void *data)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_close(function, data);
  }


  Gosu_TextInput* Gosu_Window_text_input(Gosu_Window* window)
  {
    Gosu::Window *gosu_window = reinterpret_cast<Gosu::WindowForWrapper *>( window );
    return reinterpret_cast<Gosu_TextInput*>( gosu_window->input().text_input() );
  }

  void Gosu_Window_set_text_input(Gosu_Window* window, Gosu_TextInput* text_input)
  {
    Gosu::Window *gosu_window = reinterpret_cast<Gosu::WindowForWrapper *>( window );
    gosu_window->input().set_text_input( reinterpret_cast<Gosu::TextInput*>( text_input ));
  }


  void Gosu_Window_show(Gosu_Window* window)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->show();
  }

  bool Gosu_Window_tick(Gosu_Window* window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->tick();
  }

  void Gosu_Window_close_immediately(Gosu_Window* window)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->close_immediately();
  }

  bool Gosu_Window_is_fullscreen(Gosu_Window* window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->fullscreen();
  }

  bool Gosu_Window_is_resizable(Gosu_Window *window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->resizable();
  }

  const char* Gosu_Window_caption(Gosu_Window* window)
  {
    static thread_local std::string caption;
    caption = reinterpret_cast<Gosu::WindowForWrapper *>(window)->caption();

    return caption.c_str();
  }

  void Gosu_Window_set_caption(Gosu_Window* window, const char* caption)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_caption(caption);
  }

  double Gosu_Window_update_interval(Gosu_Window* window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->update_interval();
  }

  void Gosu_Window_set_update_interval(Gosu_Window* window, double update_interval)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_update_interval(update_interval);
  }

  void Gosu_Window_set_mouse_x(Gosu_Window* window, double x)
  {
    Gosu::Window* gosu_window = reinterpret_cast<Gosu::WindowForWrapper*>( window );
    gosu_window->input().set_mouse_position(x, gosu_window->input().mouse_x());
  }

  void Gosu_Window_set_mouse_y(Gosu_Window* window, double y)
  {
    Gosu::Window* gosu_window = reinterpret_cast<Gosu::WindowForWrapper*>( window );
    gosu_window->input().set_mouse_position(gosu_window->input().mouse_x(), y);
  }

  double Gosu_Window_mouse_x(Gosu_Window* window)
  {
   return reinterpret_cast<Gosu::WindowForWrapper*>( window )->input().mouse_x();
  }

  double Gosu_Window_mouse_y(Gosu_Window* window)
  {
   return reinterpret_cast<Gosu::WindowForWrapper*>( window )->input().mouse_y();
  }

  int Gosu_Window_width(Gosu_Window* window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->width();
  }

  void Gosu_Window_set_width(Gosu_Window* window, int width)
  {
    Gosu::Window* gosu_window = reinterpret_cast<Gosu::WindowForWrapper*>( window );
    gosu_window->resize(width, gosu_window->height(), gosu_window->fullscreen());
  }

  int Gosu_Window_height(Gosu_Window* window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->height();
  }


  void Gosu_Window_set_height(Gosu_Window* window, int height)
  {
    Gosu::Window* gosu_window = reinterpret_cast<Gosu::WindowForWrapper*>( window );
    gosu_window->resize(gosu_window->width(), height, gosu_window->fullscreen());
  }

  void Gosu_Window_resize(Gosu_Window* window, int width, int height, bool fullscreen)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->resize(width, height, fullscreen);
  }

  int Gosu_Window_is_button_down(Gosu_Window* window, unsigned btn)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->input().down(Gosu::Button(btn));
  }

  // Destructor
  void Gosu_Window_destroy(Gosu_Window* window)
  {
    delete( reinterpret_cast<Gosu::WindowForWrapper*>( window ));
  }
}