#include "Gosu_FFI_internal.h"

GOSU_FFI_API void Gosu_gl_z(double z, void function(void*), void* data)
{
    Gosu::Graphics::gl(z, [=]() { function(data); });
}

GOSU_FFI_API void Gosu_gl(void function(void*), void* data)
{
    Gosu::Graphics::gl([=] { function(data); });
}

GOSU_FFI_API Gosu_Image* Gosu_render(int width, int height, void function(void*), void* data,
                                     unsigned image_flags)
{
    Gosu::Image image = Gosu::Graphics::render(width, height, [=] { function(data); }, image_flags);
    return new Gosu_Image{image};
}

GOSU_FFI_API Gosu_Image* Gosu_record(int width, int height, void function(void*), void* data)
{
    Gosu::Image image = Gosu::Graphics::record(width, height, [=] { function(data); });
    return new Gosu_Image{image};
}

GOSU_FFI_API void Gosu_flush(void)
{
    Gosu::Graphics::flush();
}

GOSU_FFI_API void Gosu_transform(double m0, double m1, double m2, double m3, double m4, double m5,
                                 double m6, double m7, double m8, double m9, double m10, double m11,
                                 double m12, double m13, double m14, double m15,
                                 void function(void*), void* data)
{
    Gosu::Transform transform = {m0, m1, m2,  m3,  m4,  m5,  m6,  m7,
                                 m8, m9, m10, m11, m12, m13, m14, m15};
    Gosu::Graphics::transform(transform, [=] { function(data); });
}

GOSU_FFI_API void Gosu_translate(double x, double y, void function(void*), void* data)
{
    Gosu::Graphics::transform(Gosu::translate(x, y), [=] { function(data); });
}

GOSU_FFI_API void Gosu_scale(double scale_x, double scale_y, double around_x, double around_y,
                             void function(void*), void* data)
{
    Gosu::Graphics::transform(Gosu::scale(scale_x, scale_y, around_x, around_y),
                              [=] { function(data); });
}

GOSU_FFI_API void Gosu_rotate(double angle, double around_x, double around_y, void function(void*),
                              void* data)
{
    Gosu::Graphics::transform(Gosu::rotate(angle, around_x, around_y), [=] { function(data); });
}

GOSU_FFI_API void Gosu_clip_to(double x, double y, double width, double height,
                               void function(void*), void* data)
{
    Gosu::Graphics::clip_to(x, y, width, height, [=] { function(data); });
}

GOSU_FFI_API void Gosu_draw_line(double x1, double y1, unsigned c1, double x2, double y2,
                                 unsigned c2, double z, unsigned mode)
{
    Gosu::Graphics::draw_line(x1, y1, c1, x2, y2, c2, z, static_cast<Gosu::BlendMode>(mode));
}

GOSU_FFI_API void Gosu_draw_triangle(double x1, double y1, unsigned c1, double x2, double y2,
                                     unsigned c2, double x3, double y3, unsigned c3, double z,
                                     unsigned mode)
{
    Gosu::Graphics::draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z,
                                  static_cast<Gosu::BlendMode>(mode));
}

GOSU_FFI_API void Gosu_draw_rect(double x, double y, double width, double height, unsigned c,
                                 double z, unsigned mode)
{
    Gosu::Graphics::draw_rect(x, y, width, height, c, z, static_cast<Gosu::BlendMode>(mode));
}

GOSU_FFI_API void Gosu_draw_quad(double x1, double y1, unsigned c1, double x2, double y2,
                                 unsigned c2, double x3, double y3, unsigned c3, double x4,
                                 double y4, unsigned c4, double z, unsigned mode)
{
    Gosu::Graphics::draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z,
                              static_cast<Gosu::BlendMode>(mode));
}

GOSU_FFI_API double Gosu_distance(double x1, double y1, double x2, double y2)
{
    return Gosu::distance(x1, y1, x2, y2);
}

GOSU_FFI_API double Gosu_angle(double from_x, double from_y, double to_x, double to_y)
{
    return Gosu::angle(from_x, from_y, to_x, to_y);
}

GOSU_FFI_API double Gosu_angle_diff(double from, double to)
{
    return Gosu::angle_diff(from, to);
}

GOSU_FFI_API double Gosu_offset_x(double angle, double radius)
{
    return Gosu::offset_x(angle, radius);
}

GOSU_FFI_API double Gosu_offset_y(double angle, double radius)
{
    return Gosu::offset_y(angle, radius);
}

GOSU_FFI_API double Gosu_random(double min, double max)
{
    return Gosu::random(min, max);
}

GOSU_FFI_API int Gosu_available_width(Gosu_Window* window)
{
    return Gosu::available_width(window);
}

GOSU_FFI_API int Gosu_available_height(Gosu_Window* window)
{
    return Gosu::available_height(window);
}

GOSU_FFI_API int Gosu_screen_width(Gosu_Window* window)
{
    return Gosu::screen_width(window);
}

GOSU_FFI_API int Gosu_screen_height(Gosu_Window* window)
{
    return Gosu::screen_height(window);
}

GOSU_FFI_API int Gosu_button_down(int id)
{
    return Gosu::Input::down(static_cast<Gosu::Button>(id));
}

GOSU_FFI_API const char* Gosu_button_id_to_char(int id)
{
    static thread_local std::string button;
    button = Gosu::Input::id_to_char(static_cast<Gosu::Button>(id));
    return button.c_str();
}

GOSU_FFI_API unsigned Gosu_button_char_to_id(const char* btn)
{
    return Gosu::Input::char_to_id(btn);
}

GOSU_FFI_API const char* Gosu_button_name(int id)
{
    static thread_local std::string name;
    name = Gosu::Input::button_name(static_cast<Gosu::Button>(id));
    return name.empty() ? nullptr : name.c_str();
}

GOSU_FFI_API const char* Gosu_gamepad_name(int id)
{
    static thread_local std::string name;
    name = Gosu::Input::gamepad_name(id);
    return name.empty() ? nullptr : name.c_str();
}

GOSU_FFI_API double Gosu_axis(int id)
{
    return Gosu::Input::axis(static_cast<Gosu::Button>(id));
}

GOSU_FFI_API int Gosu_fps()
{
    return Gosu::fps();
}

GOSU_FFI_API void Gosu_get_user_languages(void function(void*, const char*), void* data)
{
    for (const std::string& user_language : Gosu::user_languages()) {
        function(data, user_language.c_str());
    }
}

GOSU_FFI_API uint64_t Gosu_milliseconds()
{
    return Gosu::milliseconds();
}

GOSU_FFI_API const char* Gosu_default_font_name()
{
    static thread_local std::string name;
    name = Gosu::default_font_name();
    return name.c_str();
}
