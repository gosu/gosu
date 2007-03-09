#include <GosuImpl/Direct3D/Direct3D.hpp>
#include <GosuImpl/Graphics/BlockAllocator.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/WinUtility.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <stdexcept>
#include <vector>

struct Gosu::Direct3D::Texture::Impl
{
    IDirect3DDevice8* device;
    boost::shared_ptr<IDirect3DTexture8> texture;
    BlockAllocator alloc;

    Impl(unsigned size)
    : alloc(size, size)
    {
    }
};

Gosu::Direct3D::Texture::Texture(IDirect3DDevice8* device, unsigned size)
: pimpl(new Impl(size))
{
    pimpl->device = device;
    IDirect3DTexture8* tex;
    check(device->CreateTexture(size, size, 1, 0, D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED, &tex), "creating a texture");
    pimpl->texture = Win::shareComPtr(tex);
}

Gosu::Direct3D::Texture::~Texture()
{
}

IDirect3DDevice8* Gosu::Direct3D::Texture::device() const
{
    return pimpl->device;
}

IDirect3DTexture8* Gosu::Direct3D::Texture::texture() const
{
    return pimpl->texture.get();
}

unsigned Gosu::Direct3D::Texture::size() const
{
    return pimpl->alloc.width();
}

boost::optional<RECT>
    Gosu::Direct3D::Texture::allocBlock(const Bitmap& bmp)
{
    boost::optional<BlockAllocator::Block> block =
        pimpl->alloc.alloc(bmp.width(), bmp.height());
    if (!block)
        return boost::optional<RECT>();

    RECT rect;
    rect.left = block->left;
    rect.right = rect.left + block->width - 1;
    rect.top = block->top;
    rect.bottom = rect.top + block->height - 1;

    assert(rect.left >= 0);
    assert(static_cast<unsigned>(rect.right) < size());
    assert(rect.top >= 0);
    assert(static_cast<unsigned>(rect.bottom) < size());

    D3DLOCKED_RECT lockedRect;
    check(texture()->LockRect(0, &lockedRect, &rect, 0),
        "locking a texture");
    try
    {
        for (unsigned srcY = 0; srcY < block->height; ++srcY)
            for (unsigned srcX = 0; srcX < block->width; ++srcX)
            {
                Color c = bmp.getPixel(srcX, srcY);

                // IMPR: Uuuggllllyyyy! ...but works :/
                *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(lockedRect.pBits) + srcX * 4 + srcY * lockedRect.Pitch) = c.argb();
            }
    }
    catch (...)
    {
        texture()->UnlockRect(0);
        throw;
    }
    check(texture()->UnlockRect(0), "unlocking a texture");
    return rect;
}

void Gosu::Direct3D::Texture::freeBlock(unsigned x, unsigned y)
{
    pimpl->alloc.free(x, y);
}
