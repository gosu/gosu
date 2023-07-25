#include <Gosu/Buffer.hpp>
#include <Gosu/Platform.hpp>
#include <limits>
#include <memory>
#include <stdexcept>

#ifdef GOSU_IS_IPHONE
#include <filesystem>
#include <fstream>
#else
#include <SDL.h>
#endif

Gosu::Buffer::Buffer(void* buffer, std::size_t size, std::function<void(void*)> deleter)
    : m_buffer(buffer),
      m_size(size),
      m_deleter(std::move(deleter))
{
    if (buffer == nullptr && size > 0) {
        throw std::invalid_argument("Tried to create non-empty Gosu::Buffer from nullptr");
    }
    if (size >= std::numeric_limits<std::ptrdiff_t>::max()) {
        throw std::length_error("Tried to create Gosu::Buffer with a negative size");
    }
}

Gosu::Buffer::Buffer(Gosu::Buffer&& other) noexcept
    : m_buffer(nullptr),
      m_size(0),
      m_deleter(nullptr)
{
    *this = std::move(other);
}

Gosu::Buffer& Gosu::Buffer::operator=(Gosu::Buffer&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    if (m_buffer && m_deleter) {
        m_deleter(m_buffer);
    }

    m_buffer = other.m_buffer;
    m_size = other.m_size;
    // In theory, this could throw. In that case, we are fine with a full crash of the current
    // process (as this function is marked noexcept) because throwing deleters are a user error.
    m_deleter = std::move(other.m_deleter);

    other.m_buffer = nullptr;
    other.m_size = 0;
    other.m_deleter = nullptr;

    return *this;
}

Gosu::Buffer::~Buffer()
{
    if (m_buffer && m_deleter) {
        m_deleter(m_buffer);
    }
}

Gosu::Buffer::Buffer(std::size_t size)
    : m_buffer(nullptr),
      m_size(size),
      m_deleter(std::free)
{
    if (size >= std::numeric_limits<std::ptrdiff_t>::max()) {
        throw std::length_error("Tried to create Gosu::Buffer with a negative size");
    }

    if (m_size) {
        m_buffer = std::malloc(size); // NOLINT(*-no-malloc)
        // GCOV_EXCL_START: Hard to simulate out-of-memory situations.
        if (m_buffer == nullptr) {
            throw std::bad_alloc();
        }
        // GCOV_EXCL_END
    }
}

Gosu::Buffer::Buffer(std::vector<std::uint8_t>&& vector) noexcept
    : m_buffer(vector.data()),
      m_size(vector.size()),
      // Absorb the vector into this Gosu::Buffer by moving it into the deleter lambda.
      // This is safe because the vector move constructor guarantees that pointers remain stable.
      m_deleter([vector = std::move(vector)](void*) mutable { vector.clear(); })
{
}

Gosu::Buffer::Buffer(const Gosu::Buffer& other)
    : Buffer(std::vector<std::uint8_t>(other.data(), other.data() + other.size()))
{
}

Gosu::Buffer& Gosu::Buffer::operator=(const Gosu::Buffer& other)
{
    if (this == &other) {
        return *this;
    }

    // Delegate to copy constructor + move assignment operator.
    return *this = Buffer(other);
}

#ifdef GOSU_IS_IPHONE
Gosu::Buffer Gosu::load_file(const std::string& filename)
{
    Buffer buffer(std::filesystem::file_size(filename));
    char* ptr = reinterpret_cast<char*>(buffer.data());
    std::ifstream file(filename, std::ios::binary);
    // GCOV_EXCL_START: It is hard to set up a file where file_size works but reading doesn't.
    if (!file || !file.read(ptr, static_cast<std::streamoff>(buffer.size()))) {
        throw std::runtime_error("Could not read file '" + filename + "'");
    }
    // GCOV_EXCL_END
    return buffer;
}

void Gosu::save_file(const Buffer& buffer, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    if (!file || !file.write(ptr, static_cast<std::streamoff>(buffer.size()))) {
        throw std::runtime_error("Could not write file '" + filename + "'");
    }
}
#else
Gosu::Buffer Gosu::load_file(const std::string& filename)
{
    std::size_t size = 0;
    void* contents = SDL_LoadFile(filename.c_str(), &size);
    if (contents == nullptr) {
        throw std::runtime_error("Could not read '" + filename + "', error: " + SDL_GetError());
    }
    return Buffer(contents, size, &SDL_free);
}

void Gosu::save_file(const Buffer& buffer, const std::string& filename)
{
    struct RWopsDeleter
    {
        void operator()(SDL_RWops* p) const
        {
            // SDL_RWclose will crash if given nullptr.
            if (p) {
                SDL_RWclose(p);
            }
        }
    };
    std::unique_ptr<SDL_RWops, RWopsDeleter> rwops(SDL_RWFromFile(filename.c_str(), "w"));
    if (rwops == nullptr) {
        throw std::runtime_error("Could not open '" + filename + "', error: " + SDL_GetError());
    }
    std::size_t written = SDL_RWwrite(rwops.get(), buffer.data(), buffer.size(), 1);
    if (written == 0) {
        throw std::runtime_error("Could not write to '" + filename + "', error: " + SDL_GetError());
    }
}
#endif
