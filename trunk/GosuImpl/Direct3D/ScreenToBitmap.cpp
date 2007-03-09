#ifdef DRUEBER_NACHGEDACHT

#include <GosuImpl/Direct3D/Direct3D.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/WinUtility.hpp>
#include <boost/cstdint.hpp>

boost::shared_ptr<Gosu::Bitmap>
    Gosu::Direct3D::GraphicsImpl::screenToBitmap() const
{
    // Find out the size of the front buffer (= display mode of adapter).
    D3DDISPLAYMODE mode;
    check(direct3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode));

    // Create the surface which will hold a copy of the front buffer.
    IDirect3DSurface8* rawSurface;
    check(device->CreateImageSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, &rawSurface));
    boost::shared_ptr<IDirect3DSurface8> surface = Win::shareComPtr(rawSurface);

    // Copy the front buffer to the surface.
    check(device->GetFrontBuffer(surface.get()));

    // Create the bitmap we will use for the screenshot.
    boost::shared_ptr<Bitmap> result(new Bitmap);
    result->resize(mode.Width, mode.Height);

    // Lock the whole surface so we can read from it.
    D3DLOCKED_RECT rect;
    check(surface->LockRect(&rect, 0, D3DLOCK_READONLY));
    try
    {
        boost::uint8_t* rowBegin = static_cast<boost::uint8_t*>(rect.pBits);
        for (unsigned y = 0; y < mode.Height; ++y)
        {
            for (unsigned x = 0; x < mode.Width; ++x)
            {
                boost::uint8_t* pixelBegin = rowBegin + 4 * x;
                result->setPixel(x, y,
                    *reinterpret_cast<boost::uint32_t*>(pixelBegin));
            }
            rowBegin += rect.Pitch;
        }
    }
    catch (...)
    {
        surface->UnlockRect();
    }
    check(surface->UnlockRect());

    return result;
}

#endif
