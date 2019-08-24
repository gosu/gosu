#include <Gosu/Gosu.hpp>
#include <Gosu/WindowForWrapper.hpp>

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

  void Gosu_Window_button_down(Gosu_Window* window, void function())
  {
  }

  void Gosu_Window_button_up(Gosu_Window* window, void function())
  {
  }

  void Gosu_Window_button_drop(Gosu_Window* window, void function())
  {
  }

  void Gosu_Window_button_needs_redraw(Gosu_Window* window, void function())
  {
  }

  void Gosu_Window_button_needs_cursor(Gosu_Window* window, void function())
  {
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

  // Destructor
  void Gosu_Window_destroy(Gosu_Window* window)
  {
    delete( reinterpret_cast<Gosu::WindowForWrapper*>( window ));
  }
}