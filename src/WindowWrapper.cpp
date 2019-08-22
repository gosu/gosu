#include <Gosu/Gosu.hpp>

extern "C" {
  #include <Gosu/Window.h>

  // Constructor
  Gosu_Window* Gosu_Window_create(int width, int height, int fullscreen, double update_interval, int resizable)
  {
    return reinterpret_cast<Gosu_Window*>( new Gosu::Window(width, height, fullscreen, update_interval, resizable) );
  };

  // Callbacks
  void Gosu_Window_draw(Gosu_Window* window, void function())
  {
    // reinterpret_cast<Gosu::Window*>( window )->draw_callback(&function);
  }

  void Gosu_Window_update(Gosu_Window* window, void function())
  {
    // reinterpret_cast<Gosu::Window*>( window )->draw_callback(&function);
  }

  void Gosu_Window_button_down(Gosu_Window* window, void function())
  {
    // reinterpret_cast<Gosu::Window*>( window )->draw_callback(&function);
  }

  void Gosu_Window_button_up(Gosu_Window* window, void function())
  {
    // reinterpret_cast<Gosu::Window*>( window )->draw_callback(&function);
  }

  void Gosu_Window_show(Gosu_Window* window)
  {
    reinterpret_cast<Gosu::Window*>( window )->show();
  }

  void Gosu_Window_resize(Gosu_Window* window, int width, int height, int fullscreen)
  {
    reinterpret_cast<Gosu::Window*>( window )->resize(width, height, fullscreen);
  }

  int Gosu_Window_is_button_down(Gosu_Window* window, int btn)
  {
    return Gosu::Input::down((Gosu::ButtonName)btn);
  }

  // Destructor
  void Gosu_Window_destroy(Gosu_Window* window)
  {
    delete( reinterpret_cast<Gosu::Window*>( window ));
  }
}