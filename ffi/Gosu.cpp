#include <Gosu/Gosu.hpp>

extern "C" {
#include "Gosu.h"

void Gosu_gl_z(double z, void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::gl(z, callback);
}

void Gosu_gl(void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::gl(callback);
}

Gosu_Image *Gosu_render(int width, int height, void function(void *data), void *data, unsigned image_flags)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    return reinterpret_cast<Gosu_Image *>(new Gosu::Image(Gosu::Graphics::render(width, height, callback, image_flags)));
}

Gosu_Image *Gosu_record(int width, int height, void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };
    return reinterpret_cast<Gosu_Image *>(new Gosu::Image(Gosu::Graphics::record(width, height, callback)));
}

void Gosu_flush()
{
    Gosu::Graphics::flush();
}

void Gosu_transform(double m0, double m1, double m2, double m3, double m4, double m5, double m6,
                    double m7, double m8, double m9, double m10, double m11, double m12, double m13,
                    double m14, double m15, void function(void *data), void *data)
{
    Gosu::Transform transform = {
        m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15};
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::transform(transform, callback);
}

void Gosu_translate(double x, double y, void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::transform(Gosu::translate(x, y), callback);
}

void Gosu_scale(double scale_x, double scale_y, double around_x, double around_y, void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::transform(Gosu::scale(scale_x, scale_y, around_x, around_y), callback);
}

void Gosu_rotate(double angle, double around_x, double around_y, void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::transform(Gosu::rotate(angle, around_x, around_y), callback);
}

void Gosu_clip_to(double x, double y, double width, double height, void function(void *data), void *data)
{
    std::function<void ()> callback;
    callback = [=]() { function(data); };

    Gosu::Graphics::clip_to(x, y, width, height, callback);
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

double Gosu_offset_x(double angle, double radius)
{
    return Gosu::offset_x(angle, radius);
}

double Gosu_offset_y(double angle, double radius)
{
    return Gosu::offset_y(angle, radius);
}

double Gosu_random(double min, double max)
{
    return Gosu::random(min, max);
}

unsigned Gosu_available_width(Gosu_Window *window)
{
    Gosu::Window *gosu_window = nullptr;
    if (window != nullptr) {
        gosu_window = reinterpret_cast<Gosu::Window *>(window);
    }
    return Gosu::available_width(gosu_window);
}

unsigned Gosu_available_height(Gosu_Window *window)
{
    Gosu::Window *gosu_window = nullptr;
    if (window != nullptr) {
        gosu_window = reinterpret_cast<Gosu::Window *>(window);
    }
    return Gosu::available_height(gosu_window);
}

unsigned Gosu_screen_width(Gosu_Window *window)
{
    Gosu::Window *gosu_window = nullptr;
    if (window != nullptr) {
        gosu_window = reinterpret_cast<Gosu::Window *>(window);
    }
    return Gosu::screen_width(gosu_window);
}

unsigned Gosu_screen_height(Gosu_Window *window)
{
    Gosu::Window *gosu_window = nullptr;
    if (window != nullptr) {
        gosu_window = reinterpret_cast<Gosu::Window *>(window);
    }
    return Gosu::screen_height(gosu_window);
}

int Gosu_button_down(int btn)
{
    return Gosu::Input::down((Gosu::Button)btn);
}

const char *Gosu_button_id_to_char(int id)
{
    static thread_local std::string button;
    button = Gosu::Input::id_to_char((Gosu::Button)id);

    return button.c_str();
}

unsigned Gosu_button_char_to_id(const char *btn)
{
    return Gosu::Input::char_to_id(btn);
}

const char *Gosu_button_name(int btn)
{
    static thread_local std::string name;
    name = Gosu::Input::button_name((Gosu::Button)btn);
    return name.empty() ? nullptr : name.c_str();
}

const char *Gosu_gamepad_name(int id)
{
    static thread_local std::string name;
    name = Gosu::Input::gamepad_name(id);
    return name.empty() ? nullptr : name.c_str();
}

double Gosu_axis(int btn)
{
    return Gosu::Input::axis((Gosu::Button)btn);
}

int Gosu_fps()
{
    return Gosu::fps();
}

const char *Gosu_language()
{
    static thread_local std::string language;
    language = Gosu::language();

    return language.c_str();
}

long Gosu_milliseconds()
{
    return Gosu::milliseconds();
}

const char *Gosu_default_font_name()
{
    static thread_local std::string name;
    name = Gosu::default_font_name();

    return name.c_str();
}

}