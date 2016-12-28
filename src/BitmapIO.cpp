#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>

#include <cstring>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR

// Work around this bug:
// https://bugs.launchpad.net/ubuntu/+source/gcc-5/+bug/1568899
#ifdef GOSU_IS_X
#define STBI_NO_SIMD
#endif

#include "stb_image.h"

namespace
{
    int read_callback(void* user, char* data, int size)
    {
        Gosu::Reader* reader = static_cast<Gosu::Reader*>(user);
        std::size_t remaining = reader->resource().size() - reader->position();
        std::size_t actual_size = (size < remaining ? size : remaining);
        reader->read(data, actual_size);
        return actual_size;
    }

    void skip_callback(void* user, int n)
    {
        Gosu::Reader* reader = static_cast<Gosu::Reader*>(user);
        reader->set_position(reader->position() + n);
    }

    int eof_callback(void* user)
    {
        Gosu::Reader* reader = static_cast<Gosu::Reader*>(user);
        return reader->position() == reader->resource().size();
    }

    bool is_bmp(Gosu::Reader reader)
    {
        std::size_t remaining = reader.resource().size() - reader.position();
        if (remaining < 2)
            return false;
        char magic_bytes[2];
        reader.read(magic_bytes, sizeof magic_bytes);
        reader.seek(sizeof magic_bytes);
        return magic_bytes[0] == 'B' && magic_bytes[1] == 'M';
    }
}

void Gosu::load_image_file(Gosu::Bitmap& bitmap, const std::wstring& filename)
{
    Buffer buffer;
    load_file(buffer, filename);
    load_image_file(bitmap, buffer.front_reader());
}

void Gosu::load_image_file(Gosu::Bitmap& bitmap, Reader input)
{
    bool needs_color_key = is_bmp(input);
    
    stbi_io_callbacks callbacks;
    callbacks.read = read_callback;
    callbacks.skip = skip_callback;
    callbacks.eof = eof_callback;
    int x, y, n;
    stbi_uc* bytes = stbi_load_from_callbacks(&callbacks, &input, &x, &y, &n, STBI_rgb_alpha);

    if (bytes == nullptr) {
        // TODO - stbi_failure_reason is not thread safe. Everything here should be wrapped in a mutex.
        throw std::runtime_error("Cannot load image: " + std::string(stbi_failure_reason()));
    }

    bitmap.resize(x, y);
    std::memcpy(bitmap.data(), bytes, x * y * sizeof(Gosu::Color));

    stbi_image_free(bytes);
    
    if (needs_color_key)
        apply_color_key(bitmap, Gosu::Color::FUCHSIA);
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// TODO: Move into proper internal header
namespace Gosu { bool is_extension(const wchar_t* str, const wchar_t* ext); }

void Gosu::save_image_file(const Gosu::Bitmap& bitmap, const std::wstring& filename)
{
    int ok;
    std::string utf8 = Gosu::wstring_to_utf8(filename);
    
    if (is_extension(filename.c_str(), L"bmp"))
    {
        ok = stbi_write_bmp(utf8.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data());
    }
    else if (is_extension(filename.c_str(), L"tga"))
    {
        ok = stbi_write_tga(utf8.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data());
    }
    else
    {
        ok = stbi_write_png(utf8.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data(), 0);
    }
    
    if (ok == 0)
        throw std::runtime_error("Could not save image data to file: " + utf8);
}

namespace
{
    void stbi_write_to_writer(void *context, void *data, int size)
    {
        Gosu::Writer *writer = reinterpret_cast<Gosu::Writer*>(context);
        writer->write(data, size);
    }
}

void Gosu::save_image_file(const Gosu::Bitmap& bitmap, Gosu::Writer writer,
    const std::wstring& format_hint)
{
    int ok;
    
    if (is_extension(format_hint.c_str(), L"bmp"))
    {
        ok = stbi_write_bmp_to_func(stbi_write_to_writer, &writer,
                                    bitmap.width(), bitmap.height(), 4, bitmap.data());
    }
    else if (is_extension(format_hint.c_str(), L"tga"))
    {
        stbi_write_tga_with_rle = 0;
        ok = stbi_write_tga_to_func(stbi_write_to_writer, &writer,
                                    bitmap.width(), bitmap.height(), 4, bitmap.data());
    }
    else
    {
        ok = stbi_write_png_to_func(stbi_write_to_writer, &writer,
                                    bitmap.width(), bitmap.height(), 4, bitmap.data(), 0);
    }
    
    if (ok == 0)
        throw std::runtime_error("Could not save image data to memory (format hint = '" +
                                 Gosu::wstring_to_utf8(format_hint) + "'");
}
