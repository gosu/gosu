#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <vector>
#include <freeimage.h>

namespace
{
    const int GOSU_FIFLAGS = (JPEG_EXIFROTATE | ICO_MAKEALPHA | PNG_IGNOREGAMMA);
    
    Gosu::Bitmap fibToBitmap(FIBITMAP* fib)
    {
        FreeImage_ConvertTo32Bits(fib);
        Gosu::Bitmap bitmap;
        bitmap.resize(FreeImage_GetWidth(fib), FreeImage_GetHeight(fib));
        std::memcpy(bitmap.data(), FreeImage_GetBits(fib), bitmap.width() * bitmap.height() * 4);
        FreeImage_Unload(fib);
        return bitmap;
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
    return fibToBitmap(fib);
}

Gosu::Bitmap Gosu::loadImageFile(Gosu::Reader input)
{
    // Read all available input
    std::vector<BYTE> data(input.resource().size() - input.position());
    input.read(&data[0], data.size());
    FIMEMORY* fim = FreeImage_OpenMemory(&data[0], data.size());
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);
    FIBITMAP* fib = FreeImage_LoadFromMemory(fif, fim, GOSU_FIFLAGS);
    return fibToBitmap(fib);
}

void Gosu::saveImageFile(const Bitmap& bitmap, const std::wstring& filename)
{
    // TODO
}

void Gosu::saveImageFile(const Bitmap& bitmap, Gosu::Writer writer,
    const std::wstring& formatHint)
{
    // TODO
}

Gosu::Writer Gosu::saveToPNG(const Gosu::Bitmap& bitmap, Gosu::Writer writer)
{
    saveImageFile(bitmap, writer);
    return writer.resource().backWriter();
}
