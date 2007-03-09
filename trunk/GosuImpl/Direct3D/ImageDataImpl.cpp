#include <GosuImpl/Direct3D/Direct3D.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>

Gosu::Direct3D::ImageDataImpl::ImageDataImpl()
{
}

Gosu::Direct3D::ImageDataImpl::~ImageDataImpl()
{
    if (texture)
        texture->freeBlock(texX, texY);

    // IMPR: We should signal the queue that it should cancel all blits from
    // this texture for extra safety.
}

std::auto_ptr<Gosu::ImageData>
    Gosu::Direct3D::ImageDataImpl::tryToCreate(Graphics& graphics, BlitQueue<Blit>& queue,
        const Bitmap& source, const boost::shared_ptr<Texture>& texture,
        unsigned paddingLeft, unsigned paddingRight, unsigned paddingTop,
        unsigned paddingBottom)
{
    boost::optional<RECT> rect;
    rect = texture->allocBlock(source);
    // No space for the bitmap :(
    if (!rect)
        return std::auto_ptr<ImageData>();

    // If anything happens, we have to free the allocated block manually.
    // IMPR: RAII block handle class?
    std::auto_ptr<ImageDataImpl> impl;
    try
    {
        impl.reset(new ImageDataImpl);
        impl->graphics = &graphics;
        impl->queue = &queue;
        impl->blockWidth = source.width() - paddingLeft - paddingRight;
        impl->blockHeight = source.height() - paddingTop - paddingBottom;
        impl->texX = rect->left;
        impl->texY = rect->top;

        // Transform absolute pixel positions into u/v values.
        impl->srcX  = (rect->left   + paddingLeft   + 0.0) / texture->size();
        impl->srcX2 = (rect->right  - paddingRight  + 1.0) / texture->size();
        impl->srcY  = (rect->top    + paddingTop    + 0.0) / texture->size();
        impl->srcY2 = (rect->bottom - paddingBottom + 1.0) / texture->size();
    }
    catch (...)
    {
        texture->freeBlock(rect->left, rect->top);
        throw;
    }

    // Nothing threw. Now it's *impl's responsibility to erase the bitmap
    // later.
    impl->texture = texture;

    // Done!
    return impl;
}

unsigned Gosu::Direct3D::ImageDataImpl::width() const
{
    return blockWidth;
}

unsigned Gosu::Direct3D::ImageDataImpl::height() const
{
    return blockHeight;
}

void Gosu::Direct3D::ImageDataImpl::draw(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    double x4, double y4, Color c4, ZPos z, AlphaMode mode) const
{
    float factorX = graphics->factorX();
    float factorY = graphics->factorY();

    Blit blit;
    blit.device = texture->device();
    blit.texture = texture->texture();
    blit.mode = mode;
    blit.type = Blit::btQuad;
    blit.vertices[0].x = x1 * factorX - 0.5;
    blit.vertices[0].y = y1 * factorY - 0.5;
    blit.vertices[0].z = 0.5;
    blit.vertices[0].rhw = 1;
    blit.vertices[0].diffuse = c1.argb();
    blit.vertices[0].u = srcX;
    blit.vertices[0].v = srcY;
    blit.vertices[1].x = x2 * factorX - 0.5;
    blit.vertices[1].y = y2 * factorY - 0.5;
    blit.vertices[1].z = 0.5;
    blit.vertices[1].rhw = 1;
    blit.vertices[1].diffuse = c2.argb();
    blit.vertices[1].u = srcX2;
    blit.vertices[1].v = srcY;
    blit.vertices[2].x = x3 * factorX - 0.5;
    blit.vertices[2].y = y3 * factorY - 0.5;
    blit.vertices[2].z = 0.5;
    blit.vertices[2].rhw = 1;
    blit.vertices[2].diffuse = c3.argb();
    blit.vertices[2].u = srcX;
    blit.vertices[2].v = srcY2;
    blit.vertices[3].x = x4 * factorX - 0.5;
    blit.vertices[3].y = y4 * factorY - 0.5;
    blit.vertices[3].z = 0.5;
    blit.vertices[3].rhw = 1;
    blit.vertices[3].diffuse = c4.argb();
    blit.vertices[3].u = srcX2;
    blit.vertices[3].v = srcY2;

    queue->addBlit(blit, z);
}
