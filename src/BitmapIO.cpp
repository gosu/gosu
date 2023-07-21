#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>
#include <stdexcept> // for std::runtime_error

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR // we don't really care about this, but it avoids warnings

#include <stb_image.h>

namespace
{
    constexpr int JPEG_QUALITY = 80;

    /// Returns a copy of the given Gosu::Image with the alpha channel removed.
    /// Every pixel with alpha==0 will be replaced by Gosu::Color::FUCHSIA.
    std::vector<Gosu::Color::Channel> remove_alpha_channel(const Gosu::Bitmap& bmp)
    {
        std::vector<Gosu::Color::Channel> rgb(
            static_cast<std::size_t>(bmp.width() * bmp.height() * 3));
        for (std::size_t offset = 0; offset < rgb.size(); offset += 3) {
            const Gosu::Color& color = bmp.data()[offset / 3];
            if (color.alpha == 0) {
                rgb[offset + 0] = 0xff;
                rgb[offset + 1] = 0x00;
                rgb[offset + 2] = 0xff;
            }
            else {
                rgb[offset + 0] = color.red;
                rgb[offset + 1] = color.green;
                rgb[offset + 2] = color.blue;
            }
        }
        return rgb;
    }
}

Gosu::Bitmap Gosu::load_image_file(const std::string& filename)
{
    return load_image(load_file(filename));
}

Gosu::Bitmap Gosu::load_image(const Buffer& buffer)
{
    int x = 0, y = 0, n = 0;
    stbi_uc* bytes = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()), &x, &y,
                                           &n, STBI_rgb_alpha);
    if (bytes == nullptr) {
        throw std::runtime_error("Cannot load image: " + std::string(stbi_failure_reason()));
    }

    int pixels = x * y;
    Buffer pixel_buffer(bytes, static_cast<std::size_t>(pixels) * sizeof(Color), &stbi_image_free);
    Gosu::Bitmap bitmap(x, y, std::move(pixel_buffer));

    // If we just read a BMP file, we want to apply a color key.
    if (buffer.size() > 2 && *buffer.data() == 'B' && *(buffer.data() + 1) == 'M') {
        apply_color_key(bitmap, Gosu::Color::FUCHSIA);
    }

    return bitmap;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void Gosu::save_image_file(const Gosu::Bitmap& bitmap, const std::string& filename)
{
    int ok;

    if (has_extension(filename, "bmp")) {
        ok = stbi_write_bmp(filename.c_str(), bitmap.width(), bitmap.height(), 3,
                            remove_alpha_channel(bitmap).data());
    }
    else if (has_extension(filename, "jpg") || has_extension(filename, "jpeg")) {
        ok = stbi_write_jpg(filename.c_str(), bitmap.width(), bitmap.height(), 3,
                            remove_alpha_channel(bitmap).data(), JPEG_QUALITY);
    }
    else {
        ok = stbi_write_png(filename.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data(), 0);
    }

    if (ok == 0) {
        throw std::runtime_error("Could not save image data to file: " + filename);
    }
}

Gosu::Buffer Gosu::save_image(const Gosu::Bitmap& bitmap, std::string_view format_hint)
{
    const auto stbi_write_to_vector = [](void* context, void* data, int size) {
        auto* vec = static_cast<std::vector<std::uint8_t>*>(context);
        const auto* begin = static_cast<const std::uint8_t*>(data);
        const auto* end = begin + size;
        vec->insert(vec->end(), begin, end);
    };

    std::vector<std::uint8_t> vector;
    int ok;

    if (format_hint == "bmp" || has_extension(format_hint, "bmp")) {
        ok = stbi_write_bmp_to_func(stbi_write_to_vector, &vector, bitmap.width(), bitmap.height(),
                                    3, remove_alpha_channel(bitmap).data());
    }
    else if (format_hint == "jpg" || format_hint == "jpeg" || has_extension(format_hint, "jpg")
             || has_extension(format_hint, "jpeg")) {
        ok = stbi_write_jpg_to_func(stbi_write_to_vector, &vector, bitmap.width(), bitmap.height(),
                                    3, remove_alpha_channel(bitmap).data(), JPEG_QUALITY);
    }
    else {
        ok = stbi_write_png_to_func(stbi_write_to_vector, &vector, bitmap.width(), bitmap.height(),
                                    4, bitmap.data(), 0);
    }

    if (ok == 0) {
        throw std::runtime_error("Could not save image data to memory (format hint = '"
                                 + std::string(format_hint) + "'");
    }

    return Buffer(std::move(vector));
}
