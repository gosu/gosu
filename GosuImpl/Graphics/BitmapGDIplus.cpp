#include <Gosu/Graphics.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/WinUtility.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include <objidl.h>
#include <gdiplus.h>
#include <windows.h>

namespace
{
    bool initialized = false;
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;

    // TODO: Merge with BitmapFreeImage.cpp, somehow
    void reshuffleBitmap(Gosu::Bitmap& bitmap)
    {
        // Since GDI+ only supports ARGB=BGRA formats, we
        // manually exchange the R and B channels to get to ABGR=RGBA.
        boost::uint32_t* p = reinterpret_cast<boost::uint32_t*>(bitmap.data());
        for (int i = bitmap.width() * bitmap.height(); i > 0; --i, ++p)
            *p = (*p & 0xff00ff00) | ((*p << 16) & 0x00ff0000) | ((*p >> 16) & 0x000000ff);
    }    

    void check(Gdiplus::Status status, const char* action)
    {
        if (status != Gdiplus::Ok)
            throw std::runtime_error(std::string("A GDI+ error occured while ") + action);
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

    void gdiPlusToGosu(Gosu::Bitmap& result, Gdiplus::Bitmap& bitmap)
    {
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
        
        reshuffleBitmap(result);
                
        if (guid == Gdiplus::ImageFormatBMP)
            applyColorKey(result, Gosu::Color::FUCHSIA);
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
    
    CLSID& encoderFromMimeType(const std::wstring& mimeType)
    {
        static std::map<std::wstring, CLSID> cache;
        if (cache.count(mimeType))
            return cache[mimeType];

        UINT num = 0, size = 0;
        check(Gdiplus::GetImageEncodersSize(&num, &size), "counting encoders");
        // Do the eleet int-based ceil(size / sizeof)
        unsigned vecSize = (size + sizeof(Gdiplus::ImageCodecInfo) - 1) / sizeof(Gdiplus::ImageCodecInfo);
        std::vector<Gdiplus::ImageCodecInfo> codecs(vecSize);
        check(Gdiplus::GetImageEncoders(num, size,
            &codecs[0]), "enumerating encoders");
        for (int i = 0; i < num; ++i)
            if (codecs[i].MimeType == mimeType)
                return cache[mimeType] = codecs[i].Clsid;
        throw std::runtime_error("No encoder found for " + Gosu::wstringToUTF8(mimeType));
    }

    CLSID encoderFromHint(const std::wstring& formatHint)
    {
        std::wstring::size_type idx = formatHint.rfind('.');
        std::wstring mimeType = L"image/";
        if (idx == std::wstring::npos)
            mimeType += formatHint;
        else
            mimeType += formatHint.substr(idx + 1);
        boost::to_lower(mimeType);

        // Fix pitfalls
        if (mimeType == L"image/jpg")
            mimeType = L"image/jpeg";
        else if (mimeType == L"image/tif")
            mimeType = L"image/tiff";
        
        return encoderFromMimeType(mimeType);
    }
}

void Gosu::loadImageFile(Gosu::Bitmap& result, const std::wstring& filename)
{
    requireGDIplus();

    Gdiplus::Bitmap bitmap(filename.c_str());
    check(bitmap.GetLastStatus(), ("loading " + wstringToUTF8(filename)).c_str());
    gdiPlusToGosu(result, bitmap);
}

void Gosu::loadImageFile(Gosu::Bitmap& result, Reader reader)
{
    requireGDIplus();

    boost::shared_ptr<IStream> stream = readToIStream(reader);
    Gdiplus::Bitmap bitmap(stream.get());
    check(bitmap.GetLastStatus(), "loading a bitmap from memory");
    gdiPlusToGosu(result, bitmap);
}

void Gosu::saveImageFile(const Bitmap& bitmap, const std::wstring& filename)
{
    requireGDIplus();

    Bitmap input = bitmap;
    if (boost::iends_with(filename, "bmp"))
        unapplyColorKey(input, Color::FUCHSIA);
    reshuffleBitmap(input);
    Gdiplus::Bitmap output(input.width(), input.height(), input.width() * 4,
        PixelFormat32bppARGB, (BYTE*)input.data());
    check(output.GetLastStatus(), "creating a bitmap in memory");

    check(output.Save(filename.c_str(), &encoderFromHint(filename)),
        ("writing to " + wstringToUTF8(filename)).c_str());
}

void Gosu::saveImageFile(const Bitmap& bitmap, Writer writer, const std::wstring& formatHint)
{
    requireGDIplus();

    Bitmap input = bitmap;
    if (boost::iends_with(formatHint, "bmp"))
        unapplyColorKey(input, Color::FUCHSIA);
    reshuffleBitmap(input);
    Gdiplus::Bitmap output(input.width(), input.height(), input.width() * 4,
        PixelFormat32bppARGB, (BYTE*)input.data());
    check(output.GetLastStatus(), "creating a bitmap in memory");
    
    IStream* stream = NULL;
    if (CreateStreamOnHGlobal(0, TRUE, &stream) != S_OK)
        throw std::runtime_error("Could not create IStream for writing");
    boost::shared_ptr<IStream> streamGuard(Gosu::Win::shareComPtr(stream));
    check(output.Save(stream, &encoderFromHint(formatHint)),
        "saving a bitmap to memory");

    HGLOBAL buffer;
    GetHGlobalFromStream(stream, &buffer);
    void* bufferPtr = GlobalLock(buffer);
    if (!bufferPtr)
        Win::throwLastError();
    writer.write(bufferPtr, GlobalSize(buffer));
    GlobalUnlock(buffer);
}
