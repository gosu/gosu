#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <vector>
#include <boost/cstdint.hpp>
#include <freeimage.h>

namespace
{
    const int GOSU_FIFLAGS = (JPEG_EXIFROTATE | ICO_MAKEALPHA | PNG_IGNOREGAMMA);
    
    void reshuffleBitmap(Gosu::Bitmap& bitmap)
    {
        // Since FreeImage gracefully ignores the MASK parameters above, we
        // manually exchange the R and B channels.
        boost::uint32_t* p = reinterpret_cast<boost::uint32_t*>(bitmap.data());
        for (int i = bitmap.width() * bitmap.height(); i > 0; --i, ++p)
            *p = (*p & 0xff00ff00) | ((*p << 16) & 0x00ff0000) | ((*p >> 16) & 0x000000ff);
    }
    
    void removeAlphaChannel(Gosu::Bitmap& bitmap, Gosu::Color color)
    {
        Gosu::Color* p = bitmap.data();
        for (int i = bitmap.width() * bitmap.height(); i > 0; --i, ++p)
            if (p->alpha() == 0)
                *p = color;
            else
                p->setAlpha(255);
    }
    
    Gosu::Bitmap fibToBitmap(FIBITMAP* fib, FREE_IMAGE_FORMAT fif)
    {
        Gosu::Bitmap bitmap;
        bitmap.resize(FreeImage_GetWidth(fib), FreeImage_GetHeight(fib));
        FreeImage_ConvertToRawBits(reinterpret_cast<BYTE*>(bitmap.data()),
            fib, bitmap.width() * 4, 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
        FreeImage_Unload(fib);
        reshuffleBitmap(bitmap);
        if (fif == FIF_BMP)
            Gosu::applyColorKey(bitmap, Gosu::Color::FUCHSIA);
        return bitmap;
    }
    
    FIBITMAP* bitmapToFIB(Gosu::Bitmap bitmap, FREE_IMAGE_FORMAT fif)
    {
        reshuffleBitmap(bitmap);
        if (fif == FIF_BMP)
            removeAlphaChannel(bitmap, Gosu::Color::FUCHSIA);
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

Gosu::Bitmap Gosu::loadImageFile(const std::wstring& filename)
{
    #ifdef GOSU_IS_WIN
    FREE_IMAGE_FORMAT fif = GetFileTypeU(filename.c_str());
    FIBITMAP* fib = FreeImage_LoadU(filename.c_str(), GOSU_FIFLAGS);
    #else
    std::string utf8Filename = wstringToUTF8(filename);
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(utf8Filename.c_str());
    FIBITMAP* fib = FreeImage_Load(fif, utf8Filename.c_str(), GOSU_FIFLAGS);
    #endif

    return fibToBitmap(fib, fif);
}

Gosu::Bitmap Gosu::loadImageFile(Gosu::Reader input)
{
    // Read all available input
    std::vector<BYTE> data(input.resource().size() - input.position());
    input.read(&data[0], data.size());
    FIMEMORY* fim = FreeImage_OpenMemory(&data[0], data.size());
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);
    FIBITMAP* fib = FreeImage_LoadFromMemory(fif, fim, GOSU_FIFLAGS);

    return fibToBitmap(fib, fif);
}

void Gosu::saveImageFile(const Bitmap& bitmap, const std::wstring& filename)
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

void Gosu::saveImageFile(const Bitmap& bitmap, Gosu::Writer writer,
    const std::wstring& formatHint)
{
    std::string utf8FormatHint = wstringToUTF8(formatHint);
    FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(utf8FormatHint.c_str());
    FIBITMAP* fib = bitmapToFIB(bitmap, fif);
    
    FreeImageIO fio = { NULL, WriteProc, SeekProc, TellProc };
    FreeImage_SaveToHandle(fif, fib, &fio, &writer);
    FreeImage_Unload(fib);
}
