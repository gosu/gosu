#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/WinUtility.hpp>

#include <objidl.h>
#include <gdiplus.h>
#include <windows.h>

namespace 
{
    static ULONG_PTR gdiplusToken;

    void gdiShutdown(void)
    {
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }
}

Gosu::Bitmap Gosu::loadImageFile(Gosu::Reader reader)
{
    // Initialize GDI+
    static bool gdiInitialized = false;
    if (!gdiInitialized)
    {
        gdiInitialized = true;
        static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        atexit(gdiShutdown);
    }

    Bitmap bmp;
    
    HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, reader.resource().size());
    Win::check(buffer);
    void* bufferPtr = ::GlobalLock(buffer);
    if (!bufferPtr)
    {
        ::GlobalFree(buffer);
        check(bufferPtr);
    }
    reader.read(bufferPtr, reader.resource().size());
    
    IStream* stream = NULL;
    if (::CreateStreamOnHGlobal(buffer, TRUE, &stream) != S_OK)
    {
        ::GlobalFree(buffer);
        throw std::runtime_error("Could not create IStream");
    }
    
    Gdiplus::Bitmap img(stream, false);
    
    GUID guid;
    img.GetRawFormat(&guid);
    
    if (guid == Gdiplus::ImageFormatJPEG || guid == Gdiplus::ImageFormatPNG ||
        guid == Gdiplus::ImageFormatGIF  || guid == Gdiplus::ImageFormatBMP ||
        guid == Gdiplus::ImageFormatTIFF || guid == Gdiplus::ImageFormatIcon)
    {
        bmp.resize(img.GetWidth(), img.GetHeight());

        Gdiplus::BitmapData raw;
        Gdiplus::Rect rect(0, 0, img.GetWidth(), img.GetHeight());

        // TODO: Try with different images and formats.
        img.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &raw); //img.GetPixelFormat()
        unsigned int* pixels = (unsigned int*)raw.Scan0;

        // TODO: Gosu uses RGBA, this is ARGB. But this is a really fast..
        //memcpy(bmp.data(), pixels, (img.GetHeight() * raw.Stride));
        
        for(int y=0; y<img.GetHeight(); y++)
        {
            for(int x=0; x<img.GetWidth(); x++)
            {
                bmp.setPixel(x, y, pixels[y * raw.Stride / 4 + x]);
            }
        }
        img.UnlockBits(&raw);

        if(guid == Gdiplus::ImageFormatBMP)
            applyColorKey(bmp, Color::FUCHSIA);

        // Old, using this for benchmarking.
        /*Gdiplus::Color pixelColor;
        for (int y = 0; y != img.GetHeight(); y++)
        {
            for (int x = 0; x < img.GetWidth(); x++)
            {
                img.GetPixel(x, y, &pixelColor);
                bmp.setPixel(x, y, Color(pixelColor.GetValue()));
            }
        }*/
    }
    else
    {
        // TODO: Try to load devil, otherwise throw exception
    }
    
    return bmp;
}