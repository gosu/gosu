#include "Gosu_Image.h"
#include <Gosu/Gosu.hpp>
#include <cstring>

struct Gosu_Image
{
    Gosu::Image image;
};

GOSU_FFI_API Gosu_Image* Gosu_Image_create(const char* filename, unsigned image_flags)
{
    return new Gosu_Image{Gosu::Image{filename, image_flags}};
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_markup(const char* markup, const char* font,
                                                       double font_height, int width,
                                                       double spacing, unsigned align,
                                                       unsigned font_flags, unsigned image_flags)
{
    Gosu::Bitmap bitmap = Gosu::layout_markup(markup, font, font_height, spacing, width,
                                              static_cast<Gosu::Alignment>(align), font_flags);
    return new Gosu_Image{Gosu::Image{bitmap, image_flags}};
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_text(const char* text, const char* font,
                                                     double font_height, int width, double spacing,
                                                     unsigned align, unsigned font_flags,
                                                     unsigned image_flags)
{
    Gosu::Bitmap bitmap = Gosu::layout_text(text, font, font_height, spacing, width,
                                            static_cast<Gosu::Alignment>(align), font_flags);
    return new Gosu_Image{Gosu::Image{bitmap, image_flags}};
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_blob(void* blob, int byte_count, int columns,
                                                     int rows, unsigned image_flags)
{
    std::size_t size = columns * rows * 4;
    Gosu::Bitmap bitmap{columns, rows};

    if (byte_count == size) {
        // 32 bit per pixel, assume R8G8B8A8
        std::memcpy(bitmap.data(), blob, size);
    }
    else if (byte_count == size * sizeof(float)) {
        // 128 bit per channel, assume float/float/float/float - for Texplay compatibility.
        const float* in = static_cast<const float*>(blob);
        Gosu::Color::Channel* out = reinterpret_cast<Gosu::Color::Channel*>(bitmap.data());
        for (int i = size; i > 0; --i) {
            *out++ = static_cast<Gosu::Color::Channel>(*in++ * 255);
        }
    }
    else {
        return nullptr; // abort?
    }

    return new Gosu_Image{Gosu::Image{bitmap, image_flags}};
}

GOSU_FFI_API Gosu_Image* Gosu_Image_create_from_subimage(Gosu_Image* image, int left, int top,
                                                         int width, int height)
{
    std::unique_ptr<Gosu::ImageData> image_data =
            image->image.data().subimage(left, top, width, height);

    if (!image_data) {
        return nullptr;
    }

    return new Gosu_Image{Gosu::Image{std::move(image_data)}};
}

GOSU_FFI_API void Gosu_Image_create_from_tiles(const char* source, int tile_width, int tile_height,
                                               void function(void*, Gosu_Image*), void* data,
                                               unsigned image_flags)
{
    std::vector<Gosu::Image> gosu_images =
            Gosu::load_tiles(source, tile_width, tile_height, image_flags);

    for (Gosu::Image& img : gosu_images) {
        function(data, reinterpret_cast<Gosu_Image*>(new Gosu::Image(img)));
    }
}

GOSU_FFI_API void Gosu_Image_create_tiles_from_image(Gosu_Image* image, int tile_width,
                                                     int tile_height,
                                                     void function(void*, Gosu_Image*), void* data,
                                                     unsigned image_flags)
{
    std::vector<Gosu::Image> gosu_images =
            Gosu::load_tiles(image->image.data().to_bitmap(), tile_width, tile_height, image_flags);

    for (Gosu::Image& img : gosu_images) {
        function(data, reinterpret_cast<Gosu_Image*>(new Gosu::Image(img)));
    }
}

GOSU_FFI_API void Gosu_Image_destroy(Gosu_Image* image)
{
    delete image;
}

// Properties

GOSU_FFI_API unsigned Gosu_Image_width(Gosu_Image* image)
{
    return image->image.width();
}

GOSU_FFI_API unsigned Gosu_Image_height(Gosu_Image* image)
{
    return image->image.height();
}

GOSU_FFI_API Gosu_GLTexInfo* Gosu_Image_gl_tex_info_create(Gosu_Image* image)
{
    // Ensure that our reinterpret_cast trick doesn't cause outright crashes.
    static_assert(sizeof(Gosu::GLTexInfo) == sizeof(Gosu_GLTexInfo));
    static_assert(std::is_trivial_v<Gosu::GLTexInfo>);
    static_assert(std::is_trivial_v<Gosu_GLTexInfo>);

    Gosu::GLTexInfo* gosu_texture_info = new Gosu::GLTexInfo{*image->image.data().gl_tex_info()};
    return reinterpret_cast<Gosu_GLTexInfo*>(gosu_texture_info);
}

GOSU_FFI_API void Gosu_Image_gl_tex_info_destroy(Gosu_GLTexInfo* gl_tex_info)
{
    delete reinterpret_cast<Gosu::GLTexInfo*>(gl_tex_info);
}

// Rendering

GOSU_FFI_API void Gosu_Image_draw(Gosu_Image* image, double x, double y, double z, double scale_x,
                                  double scale_y, unsigned color, unsigned mode)
{
    image->image.draw(x, y, z, scale_x, scale_y, color, static_cast<Gosu::AlphaMode>(mode));
}

GOSU_FFI_API void Gosu_Image_draw_rot(Gosu_Image* image, double x, double y, double z, double angle,
                                      double center_x, double center_y, double scale_x,
                                      double scale_y, unsigned color, unsigned mode)
{
    image->image.draw_rot(x, y, z, angle, center_x, center_y, scale_x, scale_y, color,
                          static_cast<Gosu::AlphaMode>(mode));
}

GOSU_FFI_API void Gosu_Image_draw_as_quad(Gosu_Image* image, double x1, double y1, unsigned color1,
                                          double x2, double y2, unsigned color2, double x3,
                                          double y3, unsigned color3, double x4, double y4,
                                          unsigned color4, double z, unsigned mode)
{
    image->image.data().draw(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z,
                             static_cast<Gosu::AlphaMode>(mode));
}

// Image operations

GOSU_FFI_API void Gosu_Image_insert(Gosu_Image* image, Gosu_Image* source, int x, int y)
{
    Gosu::Bitmap bmp = source->image.data().to_bitmap();
    image->image.data().insert(bmp, x, y);
}

GOSU_FFI_API uint8_t* Gosu_Image_to_blob(Gosu_Image* image)
{
    static thread_local Gosu::Bitmap bitmap;
    bitmap = image->image.data().to_bitmap();
    return reinterpret_cast<uint8_t*>(bitmap.data());
}

GOSU_FFI_API void Gosu_Image_save(Gosu_Image* image, const char* filename)
{
    Gosu::save_image_file(image->image.data().to_bitmap(), filename);
}
