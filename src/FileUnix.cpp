#include <Gosu/Platform.hpp>
#if !defined(GOSU_IS_WIN)

#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

using namespace std;

struct Gosu::File::Impl : private Gosu::Noncopyable
{
    int fd = -1;
    void* mapping = MAP_FAILED;
    
    ~Impl()
    {
        if (fd >= 0) {
            close(fd);
        }
    }
};

Gosu::File::File(const string& filename, FileMode mode)
: pimpl(new Impl)
{
    int flags;

    switch (mode) {
    case FM_READ:
        flags = O_RDONLY;
        break;
    case FM_REPLACE:
        flags = O_RDWR | O_TRUNC | O_CREAT;
        break;
    case FM_ALTER:
        flags = O_RDWR | O_CREAT;
        break;
    default:
        throw invalid_argument("Unknown file mode: " + to_string(mode));
    }

    // TODO: Locking flags?

    pimpl->fd = open(filename.c_str(), flags, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (pimpl->fd < 0) throw runtime_error("Cannot open file " + filename);
    
    if (mode == FM_READ && size() > 0) {
        pimpl->mapping = mmap(nullptr, size(), PROT_READ, 0, pimpl->fd, 0);
    }
}

Gosu::File::~File()
{
    if (pimpl->mapping != MAP_FAILED) {
        munmap(pimpl->mapping, size());
    }
}

size_t Gosu::File::size() const
{
    // TODO: Error checking?
    return static_cast<size_t>(lseek(pimpl->fd, 0, SEEK_END));
}

void Gosu::File::resize(size_t new_size)
{
    ftruncate(pimpl->fd, new_size);
}

void Gosu::File::read(size_t offset, size_t length, void* dest_buffer) const
{
    // TODO: Bounds checks?
    if (pimpl->mapping != MAP_FAILED) {
        memcpy(dest_buffer, static_cast<const char*>(pimpl->mapping) + offset, length);
    }
    else {
        // TODO: Error checking?
        lseek(pimpl->fd, offset, SEEK_SET);
        ::read(pimpl->fd, dest_buffer, length);
    }
}

void Gosu::File::write(size_t offset, size_t length, const void* source_buffer)
{
    // TODO: Error checking?
    lseek(pimpl->fd, offset, SEEK_SET);
    ::write(pimpl->fd, source_buffer, length);
}

#endif
