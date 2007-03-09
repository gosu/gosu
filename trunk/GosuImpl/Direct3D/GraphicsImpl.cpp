#include <GosuImpl/Direct3D/Direct3D.hpp>
#include <GosuImpl/Graphics/LargeImageData.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/WinUtility.hpp>

namespace Gosu
{
    using Direct3D::check;
    using Direct3D::Blit;
    using Direct3D::Texture;
    using Direct3D::ImageDataImpl;
}

struct Gosu::Graphics::Impl
{
    double factorX, factorY;

    unsigned width, height;
    bool fullscreen;
    HWND window;
    RECT outputRect;
    boost::shared_ptr<IDirect3D8> direct3d;
    boost::shared_ptr<IDirect3DDevice8> device;
    D3DPRESENT_PARAMETERS params;

    BlitQueue<Blit> queue;

    typedef std::vector<boost::weak_ptr<Texture> > Textures;
    Textures textures;

    UINT adapter;

    // Sets a lot of render states to ensure even the dumbest driver
    // understands what we are doing.
    void setStates()
    {
        check(device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1),
            "setting the vertex shader");

        check(device->SetRenderState(D3DRS_LIGHTING, 0),
            "setting render states");
        check(device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE),
            "setting render states");
        check(device->SetRenderState(D3DRS_ALPHABLENDENABLE, 1),
            "setting render states");
        check(device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA),
            "setting render states");
        check(device->SetRenderState(D3DRS_LASTPIXEL, FALSE),
            "setting render states");

        check(device->SetTexture(0, 0),
            "setting texture stage states");
        check(device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE),
            "setting texture stage states");
        check(device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE),
            "setting texture stage states");
        check(device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE),
            "setting texture stage states");
        check(device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE),
            "setting texture stage states");
        check(device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE),
            "setting texture stage states");
        check(device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE),
            "setting texture stage states");

        device->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        device->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        device->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    }
};

Gosu::Graphics::Graphics(HWND window, const RECT& outputRect,
    const DisplayMode& mode)
: pimpl(new Impl)
{
    pimpl->width = mode.width;
    pimpl->height = mode.height;
    pimpl->fullscreen = mode.fullscreen;
    pimpl->factorX = pimpl->factorY = 1;

    pimpl->direct3d = Win::shareComPtr(::Direct3DCreate8(D3D_SDK_VERSION));

    pimpl->window = window;
    pimpl->outputRect = outputRect;
    pimpl->adapter = mode.adapter;

    D3DFORMAT format;
    if (mode.fullscreen)
    {
        switch (mode.bitDepth)
        {
        case 15:
            format = D3DFMT_X1R5G5B5;
            break;
        case 16:
            format = D3DFMT_R5G6B5;
            break;
        case 24:
            format = D3DFMT_R8G8B8;
            break;
        case 32:
            format = D3DFMT_X8R8G8B8;
            break;
        default:
            throw std::runtime_error("Invalid bit depth");
        }
    }
    else
    {
        D3DDISPLAYMODE dm;
        check(pimpl->direct3d->GetAdapterDisplayMode(mode.adapter, &dm),
            "retrieving the current display mode");
        format = dm.Format;
    }

    /*// Check and set texture format.
    check(direct3d->CheckDeviceFormat(mode.adapter, D3DDEVTYPE_HAL,
            format, 0, D3DRTYPE_TEXTURE, mode.textureFormat),
            "setting the texture format");*/

    // Prepare presentation parameters for the device.
    pimpl->params.BackBufferWidth = mode.width;
    pimpl->params.BackBufferHeight = mode.height;
    pimpl->params.BackBufferFormat = format;
    pimpl->params.BackBufferCount = 1;
    pimpl->params.MultiSampleType = D3DMULTISAMPLE_NONE;
    pimpl->params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pimpl->params.hDeviceWindow = window;
    pimpl->params.EnableAutoDepthStencil = FALSE;
    pimpl->params.Flags = 0;
    if (mode.fullscreen)
    {
        pimpl->params.Windowed = FALSE;
        pimpl->params.FullScreen_RefreshRateInHz = mode.refreshRate;
        pimpl->params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }
    else
    {
        pimpl->params.Windowed = TRUE;
        pimpl->params.FullScreen_RefreshRateInHz = 0;
        pimpl->params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }

    // Create and initialize device.
    IDirect3DDevice8* dev;
    check(pimpl->direct3d->CreateDevice(mode.adapter, D3DDEVTYPE_HAL, window,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pimpl->params, &dev),
        "creating the Direct3D device");
    pimpl->device = Win::shareComPtr(dev);
    pimpl->setStates();
}

Gosu::Graphics::~Graphics()
{
	for (unsigned i = 0; i < pimpl->textures.size(); ++i)
		if (!pimpl->textures[i].expired())
		{
			OutputDebugString(TEXT("Not all Images freed before freeing Graphics!"));
			return;
		}
}

unsigned Gosu::Graphics::width() const
{
    return pimpl->width;
}

unsigned Gosu::Graphics::height() const
{
    return pimpl->height;
}

double Gosu::Graphics::factorX() const
{
    return pimpl->factorX;
}

double Gosu::Graphics::factorY() const
{
    return pimpl->factorY;
}

double Gosu::Graphics::virtualWidth() const
{
    return pimpl->width / pimpl->factorX;
}

double Gosu::Graphics::virtualHeight() const
{
    return pimpl->height / pimpl->factorY;
}

void Gosu::Graphics::setVirtualResolution(double virtualWidth, double virtualHeight)
{
    pimpl->factorX = pimpl->width / virtualWidth;
    pimpl->factorY = pimpl->height / virtualHeight;
}

bool Gosu::Graphics::begin(Color clearWithColor)
{
    switch (pimpl->device->TestCooperativeLevel())
    {
        case D3DERR_DEVICELOST:
        {
            // No chance at all.
            return false;
        }

        case D3DERR_DEVICENOTRESET:
        {
            // In windowed mode, it is possible that the device was lost
            // because the user changed their desktop Color format - just in
            // case, update it...
            if (pimpl->params.Windowed)
            {
                D3DDISPLAYMODE currentDisplayMode;
                if (SUCCEEDED(pimpl->direct3d->GetAdapterDisplayMode(pimpl->adapter,
                    &currentDisplayMode)))
                {
                    pimpl->params.BackBufferFormat = currentDisplayMode.Format;
                }
            }

            // Now that everything *should* work, try to reset.
            if (FAILED(pimpl->device->Reset(&pimpl->params)))
                return false;
            pimpl->setStates();
        }
    };

    if (clearWithColor.alpha() != 0)
        pimpl->device->Clear(0, 0, D3DCLEAR_TARGET, clearWithColor.argb(), 1, 0);

    return SUCCEEDED(pimpl->device->BeginScene());
}

void Gosu::Graphics::end()
{
    if (!pimpl->device || pimpl->device->TestCooperativeLevel() != D3D_OK)
        return;

    pimpl->queue.performBlits();
    pimpl->device->EndScene();
    if (pimpl->params.Windowed)
        pimpl->device->Present(0, &pimpl->outputRect, 0, 0);
    else
        pimpl->device->Present(0, 0, 0, 0);
}

void Gosu::Graphics::drawLine(double x1, double y1, Color c1,
    double x2, double y2, Color c2, ZPos z, AlphaMode mode)
{
    Blit blit;
    blit.device = pimpl->device.get();
    blit.texture = 0;
    blit.mode = mode;
    blit.type = Blit::btLine;
    blit.vertices[0].x = x1 * factorX() - 0.5;
    blit.vertices[0].y = y1 * factorY() - 0.5;
    blit.vertices[0].z = 0.5;
    blit.vertices[0].rhw = 1;
    blit.vertices[0].diffuse = c1.argb();
    blit.vertices[0].u = 0;
    blit.vertices[0].v = 0;
    blit.vertices[1].x = x2 * factorX() - 0.5;
    blit.vertices[1].y = y2 * factorY() - 0.5;
    blit.vertices[1].z = 0.5;
    blit.vertices[1].rhw = 1;
    blit.vertices[1].diffuse = c2.argb();
    blit.vertices[1].u = 0;
    blit.vertices[1].v = 0;
    pimpl->queue.addBlit(blit, z);
}

void Gosu::Graphics::drawTriangle(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    ZPos z, AlphaMode mode)
{
    Blit blit;
    blit.device = pimpl->device.get();
    blit.texture = 0;
    blit.mode = mode;
    blit.type = Blit::btTri;
    blit.vertices[0].x = x1 * factorX() - 0.5;
    blit.vertices[0].y = y1 * factorY() - 0.5;
    blit.vertices[0].z = 0.5;
    blit.vertices[0].rhw = 1;
    blit.vertices[0].diffuse = c1.argb();
    blit.vertices[0].u = 0;
    blit.vertices[0].v = 0;
    blit.vertices[1].x = x2 * factorX() - 0.5;
    blit.vertices[1].y = y2 * factorY() - 0.5;
    blit.vertices[1].z = 0.5;
    blit.vertices[1].rhw = 1;
    blit.vertices[1].diffuse = c2.argb();
    blit.vertices[1].u = 0;
    blit.vertices[1].v = 0;
    blit.vertices[2].x = x3 * factorX() - 0.5;
    blit.vertices[2].y = y3 * factorY() - 0.5;
    blit.vertices[2].z = 0.5;
    blit.vertices[2].rhw = 1;
    blit.vertices[2].diffuse = c3.argb();
    blit.vertices[2].u = 0;
    blit.vertices[2].v = 0;
    pimpl->queue.addBlit(blit, z);
}

void Gosu::Graphics::drawQuad(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    double x4, double y4, Color c4, ZPos z, AlphaMode mode)
{
    Blit blit;
    blit.device = pimpl->device.get();
    blit.texture = 0;
    blit.mode = mode;
    blit.type = Blit::btQuad;
    blit.vertices[0].x = x1 * factorX() - 0.5;
    blit.vertices[0].y = y1 * factorY() - 0.5;
    blit.vertices[0].z = 0.5;
    blit.vertices[0].rhw = 1;
    blit.vertices[0].diffuse = c1.argb();
    blit.vertices[0].u = 0;
    blit.vertices[0].v = 0;
    blit.vertices[1].x = x2 * factorX() - 0.5;
    blit.vertices[1].y = y2 * factorY() - 0.5;
    blit.vertices[1].z = 0.5;
    blit.vertices[1].rhw = 1;
    blit.vertices[1].diffuse = c2.argb();
    blit.vertices[1].u = 0;
    blit.vertices[1].v = 0;
    blit.vertices[2].x = x3 * factorX() - 0.5;
    blit.vertices[2].y = y3 * factorY() - 0.5;
    blit.vertices[2].z = 0.5;
    blit.vertices[2].rhw = 1;
    blit.vertices[2].diffuse = c3.argb();
    blit.vertices[2].u = 0;
    blit.vertices[2].v = 0;
    blit.vertices[3].x = x4 * factorX() - 0.5;
    blit.vertices[3].y = y4 * factorY() - 0.5;
    blit.vertices[3].z = 0.5;
    blit.vertices[3].rhw = 1;
    blit.vertices[3].diffuse = c4.argb();
    blit.vertices[3].u = 0;
    blit.vertices[3].v = 0;
    pimpl->queue.addBlit(blit, z);
}

std::auto_ptr<Gosu::ImageData>
    Gosu::Graphics::createImage(const Bitmap& src, unsigned srcX,
        unsigned srcY, unsigned srcWidth, unsigned srcHeight,
        unsigned borderFlags)
{
    // Special case: If the texture is supposed to have hard borders,
    // is quadratic, has a size that is at least 64 pixels but less than 256
    // pixels and a power of two, create a single texture just for this image.
    if ((borderFlags & bfHard) == bfHard &&
        srcWidth == srcHeight &&
        srcWidth >= 64 && srcWidth <= 256 &&
        (srcWidth & (srcWidth - 1)) == 0)
    {
        boost::shared_ptr<Texture> texture(new Texture(pimpl->device.get(), srcWidth));
        std::auto_ptr<ImageData> data;

        // Use the source bitmap directly if the source area completely covers
        // it.
        if (srcX == 0 && srcWidth == src.width() &&
            srcY == 0 && srcHeight == src.height())
        {
            data = ImageDataImpl::tryToCreate(*this, pimpl->queue, src, texture, 0, 0, 0, 0);
        }
        else
        {
            Bitmap trimmedSrc;
            trimmedSrc.resize(srcWidth, srcHeight);
            trimmedSrc.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = ImageDataImpl::tryToCreate(*this, pimpl->queue, trimmedSrc, texture, 0, 0,
                0, 0);
        }

        if (!data.get())
            throw std::logic_error("Internal texture block allocation error");
        return data;
    }

    // Too large to fit on a single texture. 
    if (srcWidth > 253 || srcHeight > 253)
    {
        Bitmap bmp;
        bmp.resize(srcWidth, srcHeight);
        bmp.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
        std::auto_ptr<ImageData> lidi;
        lidi.reset(new LargeImageData(*this, bmp, 253, 253, borderFlags));
        return lidi;
    }

    Bitmap bmp;
    applyBorderFlags(bmp, src, srcX, srcY, srcWidth, srcHeight,
        borderFlags | bfDoubleRight | bfDoubleBottom);

    // Try to put the bitmap into one of the already allocated textures.
    for (Impl::Textures::iterator i = pimpl->textures.begin(); i != pimpl->textures.end(); ++i)
    {
        // Clean up while we're at it.
        if (i->expired())
        {
            --i;
            pimpl->textures.erase(i + 1);
            continue;
        }

        boost::shared_ptr<Texture> texture(*i);

        std::auto_ptr<ImageData> data;
        data = ImageDataImpl::tryToCreate(*this, pimpl->queue, bmp, texture, 1, 2, 1, 2);
        if (data.get())
            return data;
    }

    // All textures are full: Create a new one.

    boost::shared_ptr<Texture> texture;
    texture.reset(new Texture(pimpl->device.get(), 256));
    pimpl->textures.push_back(texture);

    std::auto_ptr<ImageData> data;
    data = ImageDataImpl::tryToCreate(*this, pimpl->queue, bmp, texture, 1, 2, 1, 2);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");
    return data;
}
