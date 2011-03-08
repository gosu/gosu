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
    void* const noMapping = reinterpret_cast<void*>(-1);
}

struct Gosu::File::Impl
{
    int fd;
    void* mapping;
    
    Impl() : fd(-1), mapping(noMapping) {}
    ~Impl()
    {
        if (fd > 0)
            close(fd);
    }
};

Gosu::File::File(const std::wstring& filename, FileMode mode)
: pimpl(new Impl)
{
    int flags;

    switch (mode)
    {
    case fmRead:
        flags = O_RDONLY;
        break;
    case fmReplace:
        flags = O_RDWR | O_TRUNC | O_CREAT;
        break;
    case fmAlter:
        flags = O_RDWR | O_CREAT;
        break;
    }

    // TODO: Locking flags?

    pimpl->fd = open(narrow(filename).c_str(), flags,
        S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (pimpl->fd < 0)
        throw std::runtime_error("Cannot open file " + narrow(filename));
    
    if (mode == fmRead && size() > 0)
        pimpl->mapping = mmap(0, size(), PROT_READ, 0, pimpl->fd, 0);
}

Gosu::File::~File()
{
    if (pimpl->mapping != noMapping)
        munmap(pimpl->mapping, size());
}

std::size_t Gosu::File::size() const
{
    // IMPR: Error checking?
    return lseek(pimpl->fd, 0, SEEK_END);
}

void Gosu::File::resize(std::size_t newSize)
{
    ftruncate(pimpl->fd, newSize);
}

void Gosu::File::read(std::size_t offset, std::size_t length,
    void* destBuffer) const
{
    // TODO: err checking
    if (pimpl->mapping != noMapping)
    {
        std::memcpy(destBuffer, static_cast<const char*>(pimpl->mapping) + offset, length);
        return;
    }
    
    // IMPR: Error checking?
    lseek(pimpl->fd, offset, SEEK_SET);
    ::read(pimpl->fd, destBuffer, length);
}

void Gosu::File::write(std::size_t offset, std::size_t length,
    const void* sourceBuffer)
{
    // IMPR: Error checking?
    lseek(pimpl->fd, offset, SEEK_SET);
    ::write(pimpl->fd, sourceBuffer, length);
}
