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

#if defined(GOSU_IS_UWP)
	pimpl->handle = ::CreateFile2(filename.c_str(), access, shareMode, NULL, NULL);
#else
	pimpl->handle = ::CreateFile(filename.c_str(), access, shareMode, 0,
		creationDisp, FILE_ATTRIBUTE_NORMAL, 0);
#endif
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
	LARGE_INTEGER size;
	bool result = ::GetFileSizeEx(pimpl->handle, &size);
	if (result)
	{
		// not sure about this
		return (std::size_t)size.QuadPart;
	}
	return 0;
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
