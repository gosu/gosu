#include <Gosu/Gosu.hpp>
#include <Gosu/Window.h>

Gosu::WindowForWrapper::WindowForWrapper(int width, int height, bool fullscreen,
                                         double update_interval, bool resizable)
: Gosu::Window(width, height, fullscreen, update_interval, resizable) {
}

void Gosu::WindowForWrapper::set_update(void function())
{
  callbacks.update = function;
}

void Gosu::WindowForWrapper::set_draw(void function())
{
  callbacks.draw = function;
}

void Gosu::WindowForWrapper::set_button_down(void function(unsigned btn))
{
  callbacks.button_down = function;
}

void Gosu::WindowForWrapper::set_button_up(void function(unsigned btn))
{
  callbacks.button_up = function;
}

void Gosu::WindowForWrapper::set_drop(void function(const char* filename))
{
  callbacks.drop = function;
}

void Gosu::WindowForWrapper::set_needs_redraw(int function())
{
  callbacks.needs_redraw = function;
}

void Gosu::WindowForWrapper::set_needs_cursor(int function())
{
  callbacks.needs_cursor = function;
}

void Gosu::WindowForWrapper::update()
{
  if (callbacks.update != nullptr)
  {
    callbacks.update();
  }
}

void Gosu::WindowForWrapper::draw()
{
  if (callbacks.draw != nullptr)
  {
    callbacks.draw();
  }
}

void Gosu::WindowForWrapper::button_down(Gosu::Button btn)
{
  if (callbacks.button_down != nullptr)
  {
    callbacks.button_down(btn.id());
  }
}

void Gosu::WindowForWrapper::button_up(Gosu::Button btn)
{
  if (callbacks.button_up != nullptr)
  {
    callbacks.button_up(btn.id());
  }
}

void Gosu::WindowForWrapper::drop(const std::string& filename)
{
  if (callbacks.drop != nullptr)
  {
    callbacks.drop(filename.c_str());
  }
}

bool Gosu::WindowForWrapper::needs_redraw() const
{
  if (callbacks.needs_redraw != nullptr)
  {
    return callbacks.needs_redraw();
  } else {
    return true;
  }
}

bool Gosu::WindowForWrapper::needs_cursor() const
{
  if (callbacks.needs_cursor != nullptr)
  {
    return callbacks.needs_cursor();
  } else {
    return false;
  }
}

extern "C" {
  #include <Gosu/Window.h>

  // Constructor
  Gosu_Window* Gosu_Window_create(int width, int height, int fullscreen, double update_interval, int resizable)
  {
    return reinterpret_cast<Gosu_Window*>( new Gosu::WindowForWrapper(width, height, fullscreen, update_interval, resizable) );
  };

  // Callbacks
  void Gosu_Window_draw(Gosu_Window* window, void function())
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_draw(function);
  }

  void Gosu_Window_update(Gosu_Window* window, void function())
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_update(function);
  }

  void Gosu_Window_button_down(Gosu_Window* window, void function(unsigned btn))
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_button_down(function);
  }

  void Gosu_Window_button_up(Gosu_Window* window, void function(unsigned btn))
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_button_up(function);
  }

  void Gosu_Window_drop(Gosu_Window* window, void function(const char* filename))
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_drop(function);
  }

  void Gosu_Window_needs_redraw(Gosu_Window* window, int function())
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_needs_redraw(function);
  }

  void Gosu_Window_needs_cursor(Gosu_Window* window, int function())
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->set_needs_cursor(function);
  }




  void Gosu_Window_show(Gosu_Window* window)
  {
    reinterpret_cast<Gosu::Window*>( window )->show();
  }

  const char* Gosu_Window_caption(Gosu_Window* window)
  {
    return reinterpret_cast<Gosu::WindowForWrapper*>( window )->caption().c_str();
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

  void Gosu_Window_set_mouse_x(Gosu_Window* window, int x)
  {
    Gosu::Window* gosu_window = reinterpret_cast<Gosu::WindowForWrapper*>( window );
    gosu_window->input().set_mouse_position(x, gosu_window->input().mouse_x());
  }

  void Gosu_Window_set_mouse_y(Gosu_Window* window, int y)
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

  void Gosu_Window_resize(Gosu_Window* window, int width, int height, int fullscreen)
  {
    reinterpret_cast<Gosu::WindowForWrapper*>( window )->resize(width, height, fullscreen);
  }

  int Gosu_Window_is_button_down(Gosu_Window* window, unsigned btn)
  {
    return reinterpret_cast<Gosu::Window*>( window )->input().down(Gosu::Button(btn));
  }

  // Destructor
  void Gosu_Window_destroy(Gosu_Window* window)
  {
    delete( reinterpret_cast<Gosu::WindowForWrapper*>( window ));
  }
}