#include <Gosu/IO.hpp>
#include <cassert>
#include <cstring>

void Gosu::Reader::read(void* dest, std::size_t length)
{
    res->read(pos, length, dest);
    seek(length);
}

void Gosu::Writer::write(const void* source, std::size_t length)
{
    // Try to resize the source if necessary.
    if (pos + length > res->size())
        res->resize(pos + length);

    res->write(pos, length, source);
    seek(length);
}

std::size_t Gosu::Buffer::size() const
{
    return buf.size();
}

void Gosu::Buffer::resize(std::size_t new_size)
{
    buf.resize(new_size);
}

void Gosu::Buffer::read(std::size_t offset, std::size_t length,
    void* dest_buffer) const
{
    assert(offset + length <= size());
    if (length)
        std::memcpy(dest_buffer, &buf[offset], length);
}

void Gosu::Buffer::write(std::size_t offset, std::size_t length,
    const void* source_buffer)
{
    assert(offset + length <= size());
    if (length)
        std::memcpy(&buf[offset], source_buffer, length);
}

void Gosu::load_file(Buffer& buffer, const std::wstring& filename)
{
    File file(filename);
    buffer.resize(file.size());
    file.read(0, buffer.size(), buffer.data());
}

void Gosu::save_file(const Buffer& buffer, const std::wstring& filename)
{
    File file(filename, FM_REPLACE);
    file.write(0, buffer.size(), buffer.data());
}
