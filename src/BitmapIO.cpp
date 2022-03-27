#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>
#include <cstring>   // for std::memcpy, std::size_t
#include <stdexcept> // for std::runtime_error

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR

#include <stb_image.h>

static int read_callback(void* user, char* data, int size)
{
    Gosu::Reader* reader = static_cast<Gosu::Reader*>(user);
    std::size_t remaining = reader->resource().size() - reader->position();
    std::size_t adjusted_size = (size < remaining ? size : remaining);
    reader->read(data, adjusted_size);
    return static_cast<int>(adjusted_size);
}

static void skip_callback(void* user, int n)
{
    Gosu::Reader* reader = static_cast<Gosu::Reader*>(user);
    reader->set_position(reader->position() + n);
}

static int eof_callback(void* user)
{
    Gosu::Reader* reader = static_cast<Gosu::Reader*>(user);
    return reader->position() == reader->resource().size();
}

static bool is_bmp(Gosu::Reader reader)
{
    std::size_t remaining = reader.resource().size() - reader.position();
    if (remaining < 2) return false;
    char magic_bytes[2];
    reader.read(&magic_bytes, sizeof magic_bytes);
    return magic_bytes[0] == 'B' && magic_bytes[1] == 'M';
}

/// Returns a copy of the given Gosu::Image with the alpha channel removed.
/// Every pixel with alpha==0 will be replaced by Gosu::Color::FUCHSIA.
static std::vector<Gosu::Color::Channel> bitmap_to_rgb(const Gosu::Bitmap& bmp)
{
    std::vector<Gosu::Color::Channel> rgb(static_cast<std::size_t>(bmp.width() * bmp.height() * 3));
    for (std::size_t offset = 0; offset < rgb.size(); offset += 3) {
        const Gosu::Color& color = bmp.data()[offset / 3];
        if (color.alpha == 0) {
            rgb[offset + 0] = 0xff;
            rgb[offset + 1] = 0x00;
            rgb[offset + 2] = 0xff;
        } else {
            rgb[offset + 0] = color.red;
            rgb[offset + 1] = color.green;
            rgb[offset + 2] = color.blue;
        }
    }
    return rgb;
}

Gosu::Bitmap Gosu::load_image_file(const std::string& filename)
{
    Buffer buffer;
    load_file(buffer, filename);
    return load_image_file(buffer.front_reader());
}

Gosu::Bitmap Gosu::load_image_file(Reader input)
{
    bool needs_color_key = is_bmp(input);

    stbi_io_callbacks callbacks{};
    callbacks.read = read_callback;
    callbacks.skip = skip_callback;
    callbacks.eof = eof_callback;
    int x = 0, y = 0, n = 0;

    stbi_uc* bytes = stbi_load_from_callbacks(&callbacks, &input, &x, &y, &n, STBI_rgb_alpha);
    if (bytes == nullptr) {
        throw std::runtime_error{"Cannot load image: " + std::string(stbi_failure_reason())};
    }

    Gosu::Bitmap bitmap{x, y};
    std::memcpy(bitmap.data(), bytes, x * y * sizeof(Gosu::Color));

    stbi_image_free(bytes);

    if (needs_color_key) {
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
                            bitmap_to_rgb(bitmap).data());
    }
    else if (has_extension(filename, "tga")) {
        ok = stbi_write_tga(filename.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data());
    }
    else {
        ok = stbi_write_png(filename.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data(), 0);
    }

    if (ok == 0) {
        throw std::runtime_error("Could not save image data to file: " + filename);
    }
}

static void stbi_write_to_writer(void* context, void* data, int size)
{
    static_cast<Gosu::Writer*>(context)->write(data, size);
}

void Gosu::save_image_file(const Gosu::Bitmap& bitmap, Gosu::Writer writer,
                           const std::string_view& format_hint)
{
    int ok;

    if (has_extension(format_hint, "bmp")) {
        ok = stbi_write_bmp_to_func(stbi_write_to_writer, &writer, bitmap.width(), bitmap.height(),
                                    3, bitmap_to_rgb(bitmap).data());
    }
    else if (has_extension(format_hint, "tga")) {
        stbi_write_tga_with_rle = 0;
        ok = stbi_write_tga_to_func(stbi_write_to_writer, &writer, bitmap.width(), bitmap.height(),
                                    4, bitmap.data());
    }
    else {
        ok = stbi_write_png_to_func(stbi_write_to_writer, &writer, bitmap.width(), bitmap.height(),
                                    4, bitmap.data(), 0);
    }

    if (ok == 0) {
        throw std::runtime_error("Could not save image data to memory (format hint = '" +
                                 std::string{format_hint} + "'");
    }
}
