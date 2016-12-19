#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include "WinUtility.hpp"
#include <windows.h>

// IMPR: Error checking

struct Gosu::File::Impl
{
    HANDLE handle;

    Impl()
    : handle(INVALID_HANDLE_VALUE)
    {
    }

    ~Impl()
    {
        if (handle != INVALID_HANDLE_VALUE)
            ::CloseHandle(handle);
    }
};

Gosu::File::File(const std::wstring& filename, FileMode mode)
: pimpl(new Impl)
{
    DWORD access;
    switch (mode)
    {
    case FM_READ:
        access = GENERIC_READ;
        break;
    case FM_REPLACE:
        access = GENERIC_WRITE;
        break;
    case FM_ALTER:
        access = GENERIC_READ | GENERIC_WRITE;
        break;
    }
    DWORD share_mode = FILE_SHARE_READ;
    DWORD creation_disp = (mode == FM_READ) ? OPEN_EXISTING : OPEN_ALWAYS;

    pimpl->handle = ::CreateFile(filename.c_str(), access, share_mode, 0,
        creation_disp, FILE_ATTRIBUTE_NORMAL, 0);
    if (pimpl->handle == INVALID_HANDLE_VALUE)
        Win::throw_last_error("opening " + Gosu::narrow(filename));
    if (mode == FM_REPLACE)
        resize(0);
}

Gosu::File::~File()
{
}

std::size_t Gosu::File::size() const
{
    return ::GetFileSize(pimpl->handle, 0);
}

void Gosu::File::resize(std::size_t new_size)
{
    if (::SetFilePointer(pimpl->handle, new_size, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        Win::throw_last_error("setting the file pointer");
    Win::check(::SetEndOfFile(pimpl->handle), "resizing a file");
}

void Gosu::File::read(std::size_t offset, std::size_t length,
    void* dest_buffer) const
{
    if (::SetFilePointer(pimpl->handle, offset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        Win::throw_last_error("setting the file pointer");
    DWORD dummy;
    Win::check(::ReadFile(pimpl->handle, dest_buffer, length, &dummy, 0));
}

void Gosu::File::write(std::size_t offset, std::size_t length,
    const void* source_buffer)
{
    if (::SetFilePointer(pimpl->handle, offset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        Win::throw_last_error("setting the file pointer");
    DWORD dummy;
    Win::check(::WriteFile(pimpl->handle, source_buffer, length, &dummy, 0));
}
