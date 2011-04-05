#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>

#include <objidl.h>
#include <gdiplus.h>
#include <gdiplusbitmap.h>
#include <windows.h>

namespace 
{
    static ULONG_PTR gdiplusToken;

    void gdiShutdown(void)
    {
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }
}

Gosu::Bitmap Gosu::loadImageFile(const std::wstring& filename)
{
    // Initialize GDI+
    static boolean gdiInitialized = false;
    if(!gdiInitialized)
    {
        gdiInitialized = true;
        static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        atexit(gdiShutdown);
    }

    Bitmap bmp;

    Gdiplus::Bitmap img(filename.c_str(), false);

    GUID guid;
    img.GetRawFormat(&guid);

    if (guid == Gdiplus::ImageFormatJPEG || guid == Gdiplus::ImageFormatPNG ||
        guid == Gdiplus::ImageFormatGIF || guid == Gdiplus::ImageFormatBMP ||
        guid == Gdiplus::ImageFormatTIFF || guid == Gdiplus::ImageFormatIcon)
    {
        bmp.resize(img.GetWidth(), img.GetHeight());

        Gdiplus::BitmapData* raw = new Gdiplus::BitmapData;
        Gdiplus::Rect rect(0, 0, img.GetWidth(), img.GetHeight());

        // I am still not sure if this is a good idea, benchmarking say 3 times faster
        // TODO: Try with different images and formats.
        img.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, raw); //img.GetPixelFormat()
        unsigned int* pixels = (unsigned int*)raw->Scan0;

        // TODO: This did work for my test jpg, but didnt for my test png - investigate.
        //memcpy(bmp.data(), pixels, (img.GetHeight() * raw->Stride + img.GetWidth()));
        for(int y=0; y<img.GetHeight(); y++)
        {
            for(int x=0; x<img.GetWidth(); x++)
            {
                bmp.setPixel(x, y, pixels[y * raw->Stride / 4 + x]);
            }
        }

        img.UnlockBits(raw);

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