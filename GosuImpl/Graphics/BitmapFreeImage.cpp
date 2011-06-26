#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Utility.hpp>
#include <vector>
#include <FreeImage.h>

// Compatibility with FreeImage <3.1.3. Subtly changes Gosu's behavior though.
#ifndef JPEG_EXIFROTATE
#define JPEG_EXIFROTATE 0
#endif

// With MSVC, add a suffix so FreeImage can be linked as a fallback for GDI+.
// With MinGW, Gosu uses FreeImage all the time, so no suffix is needed.
#ifdef _MSC_VER
#define FI(x) x##_FreeImage
#else
#define FI(x) x
#endif

namespace
{
    const int GOSU_FIFLAGS = (JPEG_EXIFROTATE | ICO_MAKEALPHA | PNG_IGNOREGAMMA);
    
    void reshuffleBitmap(Gosu::Bitmap& bitmap)
    {
        // Since FreeImage gracefully ignores the MASK parameters above, we
        // manually exchange the R and B channels.
        std::tr1::uint32_t* p = reinterpret_cast<std::tr1::uint32_t*>(bitmap.data());
        for (int i = bitmap.width() * bitmap.height(); i > 0; --i, ++p)
            *p = (*p & 0xff00ff00) | ((*p << 16) & 0x00ff0000) | ((*p >> 16) & 0x000000ff);
    }
    
    void fibToBitmap(Gosu::Bitmap& bitmap, FIBITMAP* fib, FREE_IMAGE_FORMAT fif)
    {
        bitmap.resize(FreeImage_GetWidth(fib), FreeImage_GetHeight(fib));
        FreeImage_ConvertToRawBits(reinterpret_cast<BYTE*>(bitmap.data()),
            fib, bitmap.width() * 4, 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
        FreeImage_Unload(fib);
        reshuffleBitmap(bitmap);
        if (fif == FIF_BMP)
            Gosu::applyColorKey(bitmap, Gosu::Color::FUCHSIA);
    }
    
    FIBITMAP* bitmapToFIB(Gosu::Bitmap bitmap, FREE_IMAGE_FORMAT fif)
    {
        reshuffleBitmap(bitmap);
        if (fif == FIF_BMP)
            unapplyColorKey(bitmap, Gosu::Color::FUCHSIA);
        return FreeImage_ConvertFromRawBits((BYTE*)bitmap.data(),
            bitmap.width(), bitmap.height(), bitmap.width() * 4, 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, true);
    }
    
    // Wrap Gosu::Writer as a FreeImageIO.
    unsigned DLL_CALLCONV WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
    {
        ((Gosu::Writer*)handle)->write(buffer, size * count);
        return count;
    }
    int DLL_CALLCONV SeekProc(fi_handle handle, long offset, int origin)
    {
        Gosu::Writer& writer = *(Gosu::Writer*)handle;
        switch (origin)
        {
        case SEEK_SET: writer.setPosition(offset); break;
        case SEEK_CUR: writer.seek(offset); break;
        case SEEK_END: writer.setPosition(writer.resource().size() - offset); break;
        };
        return 0;
    }
    long DLL_CALLCONV TellProc(fi_handle handle)
    {
        return ((Gosu::Writer*)handle)->position();
    }
}

// TODO: error checking w/ FreeImage_SetOutputMessage?

namespace Gosu
{
    void FI(loadImageFile)(Bitmap& bitmap, const std::wstring& filename)
    {
        #ifdef GOSU_IS_WIN
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(filename.c_str());
        FIBITMAP* fib = FreeImage_LoadU(fif, filename.c_str(), GOSU_FIFLAGS);
        #else
        std::string utf8Filename = wstringToUTF8(filename);
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(utf8Filename.c_str());
        FIBITMAP* fib = FreeImage_Load(fif, utf8Filename.c_str(), GOSU_FIFLAGS);
        #endif

        fibToBitmap(bitmap, fib, fif);
    }

    void FI(loadImageFile)(Bitmap& bitmap, Gosu::Reader input)
    {
        // Read all available input
        std::vector<BYTE> data(input.resource().size() - input.position());
        input.read(&data[0], data.size());
        FIMEMORY* fim = FreeImage_OpenMemory(&data[0], data.size());
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);
        FIBITMAP* fib = FreeImage_LoadFromMemory(fif, fim, GOSU_FIFLAGS);

        fibToBitmap(bitmap, fib, fif);
    }

    void FI(saveImageFile)(const Bitmap& bitmap, const std::wstring& filename)
    {
        std::string utf8Filename = wstringToUTF8(filename);
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(utf8Filename.c_str());
        FIBITMAP* fib = bitmapToFIB(bitmap, fif);

        #ifdef GOSU_IS_WIN
        FreeImage_SaveU(fif, fib, filename.c_str());
        #else
        FreeImage_Save(fif, fib, utf8Filename.c_str());
        #endif
        FreeImage_Unload(fib);
    }

    void FI(saveImageFile)(const Bitmap& bitmap, Gosu::Writer writer,
        const std::wstring& formatHint)
    {
        std::string utf8FormatHint = wstringToUTF8(formatHint);
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(utf8FormatHint.c_str());
        FIBITMAP* fib = bitmapToFIB(bitmap, fif);
        
        FreeImageIO fio = { NULL, WriteProc, SeekProc, TellProc };
        FreeImage_SaveToHandle(fif, fib, &fio, &writer);
        FreeImage_Unload(fib);
    }
}
