#include "Gosu_FFI_internal.h"
#include <cstring> // for std::memcpy

GOSU_FFI_API Gosu_Image* Gosu_Image_create(const char* filename, unsigned image_flags)
{
    return Gosu_translate_exceptions([=] { //
        return new Gosu_Image { Gosu::Image(filename, image_flags) };
    });
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_rect(const char* filename, //
                                                int x, int y, int width, int height,
                                                unsigned image_flags)
{
    return Gosu_translate_exceptions([=] {
        return new Gosu_Image { Gosu::Image(filename, { x, y, width, height }, image_flags) };
    });
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_markup(const char* markup, const char* font,
                                                       double font_height, int width,
                                                       double spacing, unsigned align,
                                                       unsigned font_flags, unsigned image_flags)
{
    return Gosu_translate_exceptions([=] {
        Gosu::Bitmap bitmap = Gosu::layout_markup(markup, font, font_height, spacing, width,
                                                  static_cast<Gosu::Alignment>(align), font_flags);
        return new Gosu_Image{Gosu::Image{bitmap, image_flags}};
    });
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_text(const char* text, const char* font,
                                                     double font_height, int width, double spacing,
                                                     unsigned align, unsigned font_flags,
                                                     unsigned image_flags)
{
    return Gosu_translate_exceptions([=] {
        Gosu::Bitmap bitmap = Gosu::layout_text(text, font, font_height, spacing, width,
                                                static_cast<Gosu::Alignment>(align), font_flags);
        return new Gosu_Image{Gosu::Image{bitmap, image_flags}};
    });
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_blob(void* blob, size_t byte_count, //
                                                     int columns, int rows, //
                                                     int x, int y, int width, int height,
                                                     unsigned image_flags)
{
    return Gosu_translate_exceptions([=] {
        const int pixels = columns * rows * 4;
        Gosu::Bitmap bitmap;

        if (byte_count == pixels) {
            // 32 bit per pixel, assume R8G8B8A8
            bitmap = Gosu::Bitmap(columns, rows, Gosu::Buffer(blob, byte_count, nullptr));
        }
        else if (byte_count == pixels * 4UL * sizeof(float)) {
            bitmap.resize(columns, rows);
            // 128 bit per channel, assume float/float/float/float RGBA - for Texplay compatibility.
            const float* in = static_cast<const float*>(blob);
            Gosu::Color::Channel* out = reinterpret_cast<Gosu::Color::Channel*>(bitmap.data());
            for (std::size_t i = 0; i < pixels; ++i) {
                out[i] = static_cast<Gosu::Color::Channel>(in[i] * 255);
            }
        }
        else {
            throw std::invalid_argument("Invalid byte_count " + std::to_string(byte_count)
                                        + " for image of size " + std::to_string(columns) + "x"
                                        + std::to_string(rows));
        }

        return new Gosu_Image { Gosu::Image(bitmap, { x, y, width, height }, image_flags) };
    });
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_subimage(Gosu_Image* image, int left, int top,
                                                         int width, int height)
{
    return Gosu_translate_exceptions([=]() -> Gosu_Image* {
        std::unique_ptr<Gosu::Drawable> drawable
            = image->image.drawable().subimage(Gosu::Rect { left, top, width, height });

        if (! drawable) return nullptr;

        return new Gosu_Image{Gosu::Image{std::move(drawable)}};
    });
}

GOSU_FFI_API void Gosu_Image_create_from_tiles(const char* source, int tile_width, int tile_height,
                                               void function(void*, Gosu_Image*), void* data,
                                               unsigned image_flags)
{
    Gosu_translate_exceptions([=] {
        std::vector<Gosu::Image> gosu_images =
            Gosu::load_tiles(source, tile_width, tile_height, image_flags);

        for (Gosu::Image& img : gosu_images) {
            function(data, new Gosu_Image{img});
        }
    });
}

GOSU_FFI_API void Gosu_Image_create_tiles_from_image(Gosu_Image* image, int tile_width,
                                                     int tile_height,
                                                     void function(void*, Gosu_Image*), void* data,
                                                     unsigned image_flags)
{
    Gosu_translate_exceptions([=] {
        std::vector<Gosu::Image> gosu_images = Gosu::load_tiles(
            image->image.drawable().to_bitmap(), tile_width, tile_height, image_flags);

        for (Gosu::Image& img : gosu_images) {
            function(data, new Gosu_Image{img});
        }
    });
}

GOSU_FFI_API void Gosu_Image_destroy(Gosu_Image* image)
{
    delete image;
}

// Properties

GOSU_FFI_API unsigned Gosu_Image_width(Gosu_Image* image)
{
    return Gosu_translate_exceptions([=] {
        return image->image.width();
    });
}

GOSU_FFI_API unsigned Gosu_Image_height(Gosu_Image* image)
{
    return Gosu_translate_exceptions([=] {
        return image->image.height();
    });
}

GOSU_FFI_API Gosu_GLTexInfo* Gosu_Image_gl_tex_info_create(Gosu_Image* image)
{
    return Gosu_translate_exceptions([=]() -> Gosu_GLTexInfo* {
        const Gosu::GLTexInfo* info = image->image.drawable().gl_tex_info();
        if (!info) {
            return nullptr;
        }
        return new Gosu_GLTexInfo {
            .tex_name = info->tex_name,
            .left = info->left,
            .right = info->right,
            .top = info->top,
            .bottom = info->bottom,
        };
    });
}

GOSU_FFI_API void Gosu_Image_gl_tex_info_destroy(Gosu_GLTexInfo* gl_tex_info)
{
    Gosu_translate_exceptions([=] {
        delete reinterpret_cast<Gosu::GLTexInfo*>(gl_tex_info);
    });
}

// Rendering

GOSU_FFI_API void Gosu_Image_draw(Gosu_Image* image, double x, double y, double z, double scale_x,
                                  double scale_y, unsigned color, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        image->image.draw(x, y, z, scale_x, scale_y, color, static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Image_draw_rot(Gosu_Image* image, double x, double y, double z, double angle,
                                      double center_x, double center_y, double scale_x,
                                      double scale_y, unsigned color, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        image->image.draw_rot(x, y, z, angle, center_x, center_y, scale_x, scale_y, color,
                              static_cast<Gosu::BlendMode>(mode));
    });
}

GOSU_FFI_API void Gosu_Image_draw_as_quad(Gosu_Image* image, double x1, double y1, unsigned color1,
                                          double x2, double y2, unsigned color2, double x3,
                                          double y3, unsigned color3, double x4, double y4,
                                          unsigned color4, double z, unsigned mode)
{
    Gosu_translate_exceptions([=] {
        image->image.drawable().draw(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z,
                                 static_cast<Gosu::BlendMode>(mode));
    });
}

// Image operations

GOSU_FFI_API void Gosu_Image_insert(Gosu_Image* image, Gosu_Image* source, int x, int y)
{
    Gosu_translate_exceptions([=] {
        Gosu::Bitmap bitmap = source->image.drawable().to_bitmap();
        image->image.drawable().insert(bitmap, x, y);
    });
}

GOSU_FFI_API uint8_t* Gosu_Image_to_blob(Gosu_Image* image)
{
    static thread_local Gosu::Bitmap bitmap;
    return Gosu_translate_exceptions([=] {
        bitmap = image->image.drawable().to_bitmap();
        return reinterpret_cast<uint8_t*>(bitmap.data());
    });
}

GOSU_FFI_API void Gosu_Image_save(Gosu_Image* image, const char* filename)
{
    Gosu_translate_exceptions([=] {
        Gosu::save_image_file(image->image.drawable().to_bitmap(), filename);
    });
}
