#ifdef GOSU_ENABLE_VIDEO

#include "Gosu_FFI_internal.h"

GOSU_FFI_API Gosu_Video* Gosu_Video_create(const char* filename, unsigned image_flags)
{
    return Gosu_translate_exceptions(
            [=] { return new Gosu_Video{Gosu::Video(filename, image_flags)}; });
}

GOSU_FFI_API Gosu_Video* Gosu_Video_create_scaled(const char* filename, int width, int height,
                                                  unsigned image_flags)
{
    return Gosu_translate_exceptions(
            [=] { return new Gosu_Video{Gosu::Video(filename, width, height, image_flags)}; });
}

GOSU_FFI_API unsigned Gosu_Video_width(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.width(); });
}

GOSU_FFI_API unsigned Gosu_Video_height(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.height(); });
}

GOSU_FFI_API void Gosu_Video_draw(Gosu_Video* video, double x, double y, double z, double scale_x,
                                  double scale_y, unsigned color, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        video->video.draw(x, y, z, scale_x, scale_y, color, static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Video_draw_rot(Gosu_Video* video, double x, double y, double z, double angle,
                                      double center_x, double center_y, double scale_x,
                                      double scale_y, unsigned color, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        video->video.draw_rot(x, y, z, angle, center_x, center_y, scale_x, scale_y, color,
                              static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Video_draw_as_quad(Gosu_Video* video, double x1, double y1, unsigned color1,
                                          double x2, double y2, unsigned color2, double x3,
                                          double y3, unsigned color3, double x4, double y4,
                                          unsigned color4, double z, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        video->video.draw_as_quad(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z,
                                  static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Video_pause(Gosu_Video* video)
{
    Gosu_translate_exceptions([=] { video->video.pause(); });
}

GOSU_FFI_API bool Gosu_Video_paused(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.paused(); });
}

GOSU_FFI_API void Gosu_Video_play(Gosu_Video* video, bool looping)
{
    Gosu_translate_exceptions([=] { video->video.play(looping); });
}

GOSU_FFI_API bool Gosu_Video_playing(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.playing(); });
}

GOSU_FFI_API void Gosu_Video_stop(Gosu_Video* video)
{
    Gosu_translate_exceptions([=] { video->video.stop(); });
}

GOSU_FFI_API double Gosu_Video_volume(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.volume(); });
}

GOSU_FFI_API void Gosu_Video_set_volume(Gosu_Video* video, double volume)
{
    Gosu_translate_exceptions([=] { video->video.set_volume(volume); });
}

GOSU_FFI_API double Gosu_Video_length(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.length(); });
}

GOSU_FFI_API double Gosu_Video_position(Gosu_Video* video)
{
    return Gosu_translate_exceptions([=] { return video->video.position(); });
}

GOSU_FFI_API void Gosu_Video_set_position(Gosu_Video* video, double position)
{
    Gosu_translate_exceptions([=] { video->video.set_position(position); });
}

#endif