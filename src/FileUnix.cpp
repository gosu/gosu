#include <Gosu/Platform.hpp>
#if !defined(GOSU_IS_WIN)

#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

namespace
{
    // According to my man page, Unix folks just don't trust the 0, it's got to be a crippled -1.
    void* const no_mapping = reinterpret_cast<void*>(-1);
}

struct Gosu::File::Impl
{
    int fd;
    void* mapping;
    
    Impl() : fd(-1), mapping(no_mapping) {}
    ~Impl()
    {
        if (fd > 0)
            close(fd);
    }
};

Gosu::File::File(const std::string& filename, FileMode mode)
: pimpl(new Impl)
{
    int flags;

    switch (mode)
    {
    case FM_READ:
        flags = O_RDONLY;
        break;
    case FM_REPLACE:
        flags = O_RDWR | O_TRUNC | O_CREAT;
        break;
    case FM_ALTER:
        flags = O_RDWR | O_CREAT;
        break;
    }

    // TODO: Locking flags?

    pimpl->fd = open(filename.c_str(), flags,
        S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (pimpl->fd < 0)
        throw std::runtime_error("Cannot open file " + filename);
    
    if (mode == FM_READ && size() > 0)
        pimpl->mapping = mmap(nullptr, size(), PROT_READ, 0, pimpl->fd, 0);
}

Gosu::File::~File()
{
    if (pimpl->mapping != no_mapping)
        munmap(pimpl->mapping, size());
}

std::size_t Gosu::File::size() const
{
    // IMPR: Error checking?
    return lseek(pimpl->fd, 0, SEEK_END);
}

void Gosu::File::resize(std::size_t new_size)
{
    ftruncate(pimpl->fd, new_size);
}

void Gosu::File::read(std::size_t offset, std::size_t length,
    void* dest_buffer) const
{
    // TODO: err checking
    if (pimpl->mapping != no_mapping)
    {
        std::memcpy(dest_buffer, static_cast<const char*>(pimpl->mapping) + offset, length);
        return;
    }
    
    // IMPR: Error checking?
    lseek(pimpl->fd, offset, SEEK_SET);
    ::read(pimpl->fd, dest_buffer, length);
}

void Gosu::File::write(std::size_t offset, std::size_t length,
    const void* source_buffer)
{
    // IMPR: Error checking?
    lseek(pimpl->fd, offset, SEEK_SET);
    ::write(pimpl->fd, source_buffer, length);
}

#endif
