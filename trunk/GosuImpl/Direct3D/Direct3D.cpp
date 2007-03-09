#include <GosuImpl/Direct3D/Direct3D.hpp>
#include <Gosu/Graphics.hpp>
#include <dxerr8.h>
#include <Gosu/WinUtility.hpp>
#include <stdexcept>

GOSU_NORETURN void Gosu::Direct3D::throwError(HRESULT retVal,
    const std::string& action)
{
    std::string message;
    if (action.empty())
        message = "Direct3D error: ";
    else
        message = "While " + action + ", the following Direct3D error " +
        "occured: ";
    message += ::DXGetErrorDescription8A(retVal);

    throw std::runtime_error(message);
}

namespace
{
    int formatQuality(D3DFORMAT fmt)
    {
        switch (fmt)
        {
            case D3DFMT_X8R8G8B8:
                return 3;
            case D3DFMT_R8G8B8:
                return 2;
            case D3DFMT_R5G6B5:
                return 1;
            case D3DFMT_X1R5G5B5:
                return 0;
            default:
                return -1;
        }
    }
}

boost::optional<Gosu::DisplayMode> Gosu::DisplayMode::find(unsigned width,
    unsigned height, bool fullscreen)
{
    boost::shared_ptr<IDirect3D8> direct3d;
    direct3d = Win::shareComPtr(::Direct3DCreate8(D3D_SDK_VERSION));
    if (!direct3d)
        return boost::optional<DisplayMode>();

    D3DDISPLAYMODE curMode;
    if (FAILED(direct3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &curMode)))
        return boost::optional<DisplayMode>();

    // In windowed mode, never choose anything larger than the current mode.
    if (!fullscreen && (width >= curMode.Width || height >= curMode.Height))
        return boost::optional<DisplayMode>();

    D3DDISPLAYMODE bestMode = { 0, 0, 0, D3DFMT_UNKNOWN };
    unsigned modeCount = direct3d->GetAdapterModeCount(D3DADAPTER_DEFAULT);

    for (unsigned i = 0; i < modeCount; ++i)
    {
        D3DDISPLAYMODE d3dMode;
        if (FAILED(direct3d->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3dMode)))
            continue;

        // Filter out modes which obviously don't fit.
        if (d3dMode.Width != width || d3dMode.Height != height)
            continue;

        // Check if this device format/screen mode combination is supported in
        // HAL.
        if (FAILED(direct3d->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
            d3dMode.Format, d3dMode.Format, !fullscreen)))
            continue;

        // Check if the texture format used is supported in HAL.
        if (FAILED(direct3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
            d3dMode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
            continue;

        // When in windowed mode the format and refresh rate must match the
        // current mode.
        if (!fullscreen && (d3dMode.Format != curMode.Format || 
                            d3dMode.RefreshRate != curMode.RefreshRate))
            continue;

        // The mode passed all the tests. Now test if it's even better
        // than the mode we remembered last time this happened.
        // (This is stupid: A mode with 16BPP and 120Hz should win against
        // 32BPP/60Hz...)
        if (formatQuality(d3dMode.Format) < formatQuality(bestMode.Format))
            continue;
        if (d3dMode.RefreshRate < bestMode.RefreshRate)
            continue;

        // We have a new winner!
        bestMode = d3dMode;
    }

    // It's still the old dummy mode.
    if (bestMode.Width == 0)
        return boost::optional<DisplayMode>();

    DisplayMode mode;
    mode.adapter = D3DADAPTER_DEFAULT;
    mode.width = bestMode.Width;
    mode.height = bestMode.Height;
    mode.fullscreen = fullscreen;
    switch (bestMode.Format)
    {
    case D3DFMT_X1R5G5B5:
        mode.bitDepth = 15;
        break;
    case D3DFMT_R5G6B5:
        mode.bitDepth = 16;
        break;
    case D3DFMT_R8G8B8:
        mode.bitDepth = 24;
        break;
    case D3DFMT_X8R8G8B8:
        mode.bitDepth = 32;
        break;
    default:
        // IMPR: We should have sorted it out earlier then!
        throw std::runtime_error("Best mode has unknown format");
    }
    mode.refreshRate = bestMode.RefreshRate;
    return mode;
}
