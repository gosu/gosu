#include <Gosu/Gosu.hpp>


extern "C" {
  #include <Gosu/Gosu.h>

  void Gosu_gl_z(double z, void function())
  {
    Gosu::Graphics::gl(z, *function);
  }

  void Gosu_gl(void function())
  {
    Gosu::Graphics::gl(*function);
  }

  Gosu_Image* Gosu_render(int width, int height, void function(), unsigned image_flags)
  {
    // Disabled for now, couldn't figure out how to get a Gosu::Image* from static Gosu::Image

    // return reinterpret_cast<Gosu_Image*>( Gosu::Graphics::render(width, height, *function, image_flags) );
  }

  Gosu_Image* Gosu_record(int width, int height, void function())
  {
    // return reinterpret_cast<Gosu_Image*>( Gosu::Graphics::record(width, height, *function) );
  }

  void Gosu_flush()
  {
    Gosu::Graphics::flush();
  }

  void Gosu_transform()
  {
    // Gosu::Graphics::transform();
  }

  void Gosu_translate(double x, double y, void function())
  {
    Gosu::Graphics::transform(Gosu::translate(x, y), function);
  }

  void Gosu_scale(double scale_x, double scale_y, double around_x, double around_y, void function())
  {
    Gosu::Graphics::transform(Gosu::scale(scale_x, scale_y, around_x, around_y), function);
  }

  void Gosu_rotate(double angle, double around_x, double around_y, void function())
  {
    Gosu::Graphics::transform(Gosu::rotate(angle, around_x, around_y), function);
  }

  void Gosu_clip_to(double x, double y, double width, double height, void function())
  {
    Gosu::Graphics::clip_to(x, y, width, height, function);
  }

  void Gosu_draw_line(double x1, double y1, unsigned c1,
                    double x2, double y2, unsigned c2,
                    double z, unsigned mode)
  {
    Gosu::Graphics::draw_line(x1, y1, c1, x2, y2, c2, z, (Gosu::AlphaMode)mode);
  }

  void Gosu_draw_triangle(double x1, double y1, unsigned c1,
                          double x2, double y2, unsigned c2,
                          double x3, double y3, unsigned c3,
                          double z, unsigned mode)
  {
    Gosu::Graphics::draw_triangle(x1, y1, c1,
                                  x2, y2, c2,
                                  x3, y3, c3,
                                  z, (Gosu::AlphaMode)mode);
  }

  void Gosu_draw_rect(double x, double y, double width, double height, unsigned c, double z, unsigned mode)
  {
    Gosu::Graphics::draw_rect(x, y, width, height, c, z, (Gosu::AlphaMode)mode);
  }

  void Gosu_draw_quad(double x1, double y1, unsigned c1,
                    double x2, double y2, unsigned c2,
                    double x3, double y3, unsigned c3,
                    double x4, double y4, unsigned c4,
                    double z, unsigned mode)
  {
    Gosu::Graphics::draw_quad(x1, y1, c1,
                              x2, y2, c2,
                              x3, y3, c3,
                              x4, y4, c4,
                              z, (Gosu::AlphaMode)mode);
  }

  double Gosu_distance(double x1, double y1, double x2, double y2)
  {
    return Gosu::distance(x1, y1, x2, y2);
  }

  double Gosu_angle(double from_x, double from_y, double to_x, double to_y)
  {
    return Gosu::angle(from_x, from_y, to_x, to_y);
  }

  double Gosu_angle_diff(double from, double to)
  {
    return Gosu::angle_diff(from, to);
  }

  double Gosu_offset_x(double theta, double r)
  {
    return Gosu::offset_x(theta, r);
  }

  double Gosu_offset_y(double theta, double r)
  {
    return Gosu::offset_y(theta, r);
  }

  double Gosu_random(double min, double max)
  {
    return Gosu::random(min, max);
  }

  unsigned Gosu_available_width(Gosu_Window* window)
  {
    Gosu::Window* gosu_window = nullptr;
    if (window != nullptr) {
      gosu_window = reinterpret_cast<Gosu::Window*>( window );
    }
    return Gosu::available_width(gosu_window);
  }

  unsigned Gosu_available_height(Gosu_Window* window)
  {
    Gosu::Window* gosu_window = nullptr;
    if (window != nullptr) {
      gosu_window = reinterpret_cast<Gosu::Window*>( window );
    }
    return Gosu::available_height(gosu_window);
  }

  unsigned Gosu_screen_width(Gosu_Window* window)
  {
    Gosu::Window* gosu_window = nullptr;
    if (window != nullptr) {
      gosu_window = reinterpret_cast<Gosu::Window*>( window );
    }
    return Gosu::screen_width(gosu_window);
  }

  unsigned Gosu_screen_height(Gosu_Window* window)
  {
    Gosu::Window* gosu_window = nullptr;
    if (window != nullptr) {
      gosu_window = reinterpret_cast<Gosu::Window*>( window );
    }
    return Gosu::screen_height(gosu_window);
  }

  int Gosu_button_down(int btn)
  {
    return Gosu::Input::down((Gosu::ButtonName)btn);
  }

  const char* Gosu_button_id_to_char(int id)
  {
    return Gosu::Input::id_to_char((Gosu::Button)id).c_str();
  }

  unsigned Gosu_button_char_to_id(const char* btn)
  {
    return Gosu::Input::char_to_id(btn).id();
  }

  int Gosu_fps()
  {
    return Gosu::fps();
  }

  const char* Gosu_language()
  {
    return Gosu::language().c_str();
  }

  long Gosu_milliseconds()
  {
    return Gosu::milliseconds();
  }

  const char* Gosu_default_font_name()
  {
    return Gosu::default_font_name().c_str();
  }
}