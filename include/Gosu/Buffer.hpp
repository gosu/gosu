#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Gosu
{
    /// In many places, Gosu also accepts memory buffers in place of filenames to enable
    /// interoperability with other storage systems, such as creating images from downloaded files
    /// or loading sounds from PhysFS resources.
    ///
    /// Instead of using a vector<byte> or a similar standard container, this class consists of a
    /// pointer and a deleter. This makes it easy to create Buffers from methods in other libraries
    /// that return a pointer, but require that a specific functions is used for freeing the
    /// pointer, such as std::free or SDL_free.
    class Buffer
    {
        void* m_buffer = nullptr;
        std::size_t m_size = 0;
        std::function<void(void*)> m_deleter = nullptr;

    public:
        Buffer() = default;
        /// Creates a Buffer that refers to a given portion of memory. The deleter will be called
        /// when the Buffer is freed or re-assigned.
        /// @param deleter Can be nullptr to create a non-owning view. Must not throw.
        Buffer(void* buffer, std::size_t size, std::function<void(void*)> deleter);
        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;
        ~Buffer();

        /// Creates a new buffer by allocating memory. Does not initialize this memory.
        explicit Buffer(std::size_t size);
        /// Takes ownership of the given vector.
        explicit Buffer(std::vector<std::uint8_t>&& vector) noexcept;

        /// Makes a copy of the given buffer. Avoid this if possible.
        Buffer(const Buffer& other);
        /// Makes a copy of the given buffer. Avoid this if possible.
        Buffer& operator=(const Buffer& other);

        const std::uint8_t* data() const { return static_cast<const std::uint8_t*>(m_buffer); }
        std::uint8_t* data() { return static_cast<std::uint8_t*>(m_buffer); }
        std::size_t size() const { return m_size; }
    };

    /// Loads a whole file into a buffer.
    Buffer load_file(const std::string& filename);

    /// Creates or overwrites a file with the contents of a buffer.
    void save_file(const Buffer& buffer, const std::string& filename);
}
