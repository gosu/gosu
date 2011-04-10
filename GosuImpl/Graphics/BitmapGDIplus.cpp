#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/WinUtility.hpp>
#include <boost/cstdint.hpp>
#include <objidl.h>
#include <gdiplus.h>
#include <windows.h>

namespace
{
    bool initialized = false;
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;

    void check(Gdiplus::Status status, const std::string& action)
    {
        if (status != Gdiplus::Ok)
            throw std::runtime_error("A GDI+ error occured while " + action);
    }

    void closeGDIplus()
    {
        Gdiplus::GdiplusShutdown(token);
    }

    void requireGDIplus()
    {
        if (initialized)
            return;
        initialized = true;
        check(Gdiplus::GdiplusStartup(&token, &input, NULL), "initializing GDI+");
        atexit(closeGDIplus);
    }

    Gosu::Bitmap gdiPlusToGosu(Gdiplus::Bitmap& bitmap)
    {
        Gosu::Bitmap result;
        result.resize(bitmap.GetWidth(), bitmap.GetHeight());

        GUID guid;
        check(bitmap.GetRawFormat(&guid), "getting the format GUID");
        // TODO:
        // if (guid == Gdiplus::ImageFormatJPEG || guid == Gdiplus::ImageFormatPNG ||
        //     guid == Gdiplus::ImageFormatGIF  || guid == Gdiplus::ImageFormatBMP ||
        //     guid == Gdiplus::ImageFormatTIFF || guid == Gdiplus::ImageFormatIcon)
        // else use FreeImage (via lazy linking...)

        Gdiplus::BitmapData target;
        target.Width = result.width();
        target.Height = result.height();
        target.PixelFormat = PixelFormat32bppARGB;
        target.Stride = result.width() * 4;
        target.Scan0 = result.data();

        Gdiplus::Rect rect(0, 0, bitmap.GetWidth(), bitmap.GetHeight());

        check(bitmap.LockBits(&rect,
            Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf,
            PixelFormat32bppARGB, &target), "locking bits");
        check(bitmap.UnlockBits(&target), "unlocking bits");
        
        // Swap R and B channels
        // TODO: Can this be merged with the equivalent function from BitmapFreeImage?
        boost::uint32_t* p = reinterpret_cast<boost::uint32_t*>(result.data());
        for (int i = result.width() * result.height(); i > 0; --i, ++p)
            *p = (*p & 0xff00ff00) | ((*p << 16) & 0x00ff0000) | ((*p >> 16) & 0x000000ff);
                
        if (guid == Gdiplus::ImageFormatBMP)
            applyColorKey(result, Gosu::Color::FUCHSIA);
        
        return result;
    }

    boost::shared_ptr<IStream> readToIStream(Gosu::Reader reader)
    {
        unsigned remaining = reader.resource().size() - reader.position();
        HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, remaining);
        Gosu::Win::check(buffer);
        void* bufferPtr = ::GlobalLock(buffer);
        if (!bufferPtr)
        {
            ::GlobalFree(buffer);
            Gosu::Win::throwLastError();
        }
        reader.read(bufferPtr, reader.resource().size() - reader.position());
        
        IStream* stream = NULL;
        if (::CreateStreamOnHGlobal(buffer, TRUE, &stream) != S_OK)
        {
            ::GlobalFree(buffer);
            throw std::runtime_error("Could not create IStream");
        }
        return Gosu::Win::shareComPtr(stream);
    }
}

Gosu::Bitmap Gosu::loadImageFile(const std::wstring& filename)
{
    requireGDIplus();

    Gdiplus::Bitmap bitmap(filename.c_str());
    check(bitmap.GetLastStatus(), "loading " + wstringToUTF8(filename));
    return gdiPlusToGosu(bitmap);
}

Gosu::Bitmap Gosu::loadImageFile(Reader reader)
{
    requireGDIplus();

    boost::shared_ptr<IStream> stream = readToIStream(reader);
    Gdiplus::Bitmap bitmap(stream.get());
    check(bitmap.GetLastStatus(), "loading a bitmap from memory");
    return gdiPlusToGosu(bitmap);
}

void Gosu::saveImageFile(const Bitmap& bitmap, const std::wstring& filename)
{
    //Gdiplus::Bitmap result;
    //gosuToGDIplus(bitmap, result);
    //result.Save(filename.c_str(), 0, 0);
}

void Gosu::saveImageFile(const Bitmap& bitmap, Writer writer, const std::wstring& formatHint)
{
}
