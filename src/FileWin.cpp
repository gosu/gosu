#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/WinUtility.hpp>
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
    case fmRead:
        access = GENERIC_READ;
        break;
    case fmReplace:
        access = GENERIC_WRITE;
        break;
    case fmAlter:
        access = GENERIC_READ | GENERIC_WRITE;
        break;
    }
    DWORD shareMode = FILE_SHARE_READ;
    DWORD creationDisp = (mode == fmRead) ? OPEN_EXISTING : OPEN_ALWAYS;

    pimpl->handle = ::CreateFile(filename.c_str(), access, shareMode, 0,
        creationDisp, FILE_ATTRIBUTE_NORMAL, 0);
    if (pimpl->handle == INVALID_HANDLE_VALUE)
        Win::throwLastError("opening " + Gosu::narrow(filename));
    if (mode == fmReplace)
        resize(0);
}

Gosu::File::~File()
{
}

std::size_t Gosu::File::size() const
{
    return ::GetFileSize(pimpl->handle, 0);
}

void Gosu::File::resize(std::size_t newSize)
{
    if (::SetFilePointer(pimpl->handle, newSize, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        Win::throwLastError("setting the file pointer");
    Win::check(::SetEndOfFile(pimpl->handle), "resizing a file");
}

void Gosu::File::read(std::size_t offset, std::size_t length,
    void* destBuffer) const
{
    if (::SetFilePointer(pimpl->handle, offset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        Win::throwLastError("setting the file pointer");
    DWORD dummy;
    Win::check(::ReadFile(pimpl->handle, destBuffer, length, &dummy, 0));
}

void Gosu::File::write(std::size_t offset, std::size_t length,
    const void* sourceBuffer)
{
    if (::SetFilePointer(pimpl->handle, offset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        Win::throwLastError("setting the file pointer");
    DWORD dummy;
    Win::check(::WriteFile(pimpl->handle, sourceBuffer, length, &dummy, 0));
}
