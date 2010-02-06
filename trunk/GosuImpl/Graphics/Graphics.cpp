#include <Gosu/Graphics.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOp.hpp>
#include <GosuImpl/Graphics/Texture.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <GosuImpl/Graphics/LargeImageData.hpp>
#include <GosuImpl/Graphics/Macro.hpp>
#include <Gosu/Bitmap.hpp>
#if 0
#include <boost/thread.hpp>
#endif
#include <cmath>
#include <algorithm>
#include <limits>

struct Gosu::Graphics::Impl
{
    unsigned physWidth, physHeight;
    unsigned virtWidth, virtHeight;
    double factorX, factorY;
    bool fullscreen;
    DrawOpQueueStack queues;
    typedef std::vector<boost::shared_ptr<Texture> > Textures;
    Textures textures;
    
#if 0
    boost::mutex texMutex;
#endif
};

Gosu::Graphics::Graphics(unsigned physWidth, unsigned physHeight, bool fullscreen)
: pimpl(new Impl)
{
    pimpl->virtWidth  = pimpl->physWidth  = physWidth;
    pimpl->virtHeight = pimpl->physHeight = physHeight;
    pimpl->factorX = pimpl->factorY = 1.0;
    pimpl->fullscreen = fullscreen;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, pimpl->physWidth, pimpl->physHeight);
    #ifdef GOSU_IS_IPHONE
    glOrthof(0, pimpl->physWidth, pimpl->physHeight, 0, -1, 1);
    #else
    glOrtho(0, pimpl->physWidth, pimpl->physHeight, 0, -1, 1);
    #endif
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    #ifdef GOSU_IS_IPHONE
    glTranslatef(physWidth, 0, 0);
    glRotatef(90, 0, 0, 1);
    glScalef(3.0/2, 2.0/3, 0);
    #endif

    glEnable(GL_BLEND);
    
    // Create default draw-op queue.
    pimpl->queues.resize(1);
}

Gosu::Graphics::~Graphics()
{
}

unsigned Gosu::Graphics::width() const
{
    return pimpl->virtWidth;
}

unsigned Gosu::Graphics::height() const
{
    return pimpl->virtHeight;
}

bool Gosu::Graphics::fullscreen() const
{
    return pimpl->fullscreen;
}

double Gosu::Graphics::factorX() const
{
    return pimpl->factorX;
}

double Gosu::Graphics::factorY() const
{
    return pimpl->factorY;
}

void Gosu::Graphics::setResolution(unsigned virtualWidth, unsigned virtualHeight)
{
    if (virtualWidth * virtualHeight < 1)
        throw std::invalid_argument("Invalid virtual resolution.");

    pimpl->virtWidth = virtualWidth;
    pimpl->virtHeight = virtualHeight;
    /*    
    pimpl->factorX = pimpl->factorY =
        std::min(1.0 / virtualWidth * pimpl->physWidth,
                 1.0 / virtualHeight * pimpl->physHeight);
    */
    pimpl->factorX = 1.0 / virtualWidth * pimpl->physWidth;
    pimpl->factorY = 1.0 / virtualHeight * pimpl->physHeight;
}

bool Gosu::Graphics::begin(Gosu::Color clearWithColor)
{
    // If there is a recording in process, stop it.
    pimpl->queues.resize(1);

    // Flush leftover clippings
    endClipping();

    glClearColor(clearWithColor.red()/255.0,
                 clearWithColor.green()/255.0,
                 clearWithColor.blue()/255.0,
                 clearWithColor.alpha()/255.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    return true;
}

void Gosu::Graphics::end()
{
    /*double vBarWidth = pimpl->physWidth / factorX() - width();
    if (vBarWidth > 0)
    {
        drawQuad(0, 0, 0x00000000, vBarWidth, 0, 0x00000000,
                 0, height(), 0x00000000, vBarWidth, height(), 0x00000000,
                 std::numeric_limits<double>::max());
    }
    
    double hBarHeight = pimpl->physHeight / factorY() - height();
    if (hBarHeight > 0)
    {
        drawQuad(0, 0, 0x00000000, width(), 0, 0x00000000,
                 0, hBarHeight, 0x00000000, width(), hBarHeight, 0x00000000,
                 std::numeric_limits<double>::max());
    }*/
    
    // If there is a recording in process, stop it.
    pimpl->queues.resize(1);
    
    pimpl->queues.at(0).performDrawOps();
    pimpl->queues.at(0).clear();

    glFlush();
}

void Gosu::Graphics::beginGL()
{
    if (pimpl->queues.size() > 1)
        throw std::logic_error("Custom OpenGL is not allowed while creating a macro");
    
#ifdef GOSU_IS_IPHONE
    throw std::logic_error("Custom OpenGL is unsupported on the iPhone");
#else
    pimpl->queues.at(0).performDrawOps();
    pimpl->queues.at(0).clear();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_BLEND);
#endif
}

void Gosu::Graphics::endGL()
{
#ifdef GOSU_IS_IPHONE
    throw std::logic_error("Custom OpenGL is unsupported on the iPhone");
#else
    glPopAttrib();

    // Restore matrices.
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, pimpl->physWidth, pimpl->physHeight);
    glOrtho(0, pimpl->virtWidth, pimpl->virtHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
#endif
}

void Gosu::Graphics::beginClipping(int x, int y, unsigned width, unsigned height)
{
    if (pimpl->queues.size() > 1)
        throw std::logic_error("Clipping not allowed while creating a macro");
    
    // In doubt, make the clipping region smaller than requested.
    
#ifndef GOSU_IS_IPHONE
    int physX = static_cast<int>(std::ceil(x * factorX()));
    int physY = static_cast<int>(std::ceil((0.0 + this->height() - y - height) * factorY()));
    unsigned physWidth  = static_cast<unsigned>(width  * factorX());
    unsigned physHeight = static_cast<unsigned>(height * factorY());
#else
    // Make up for rotation
    int physX = 320 - static_cast<int>(std::ceil(320.0 * (0.0 + y + height) / this->height()));
    int physY = 480 - static_cast<int>(std::ceil(480.0 * (0.0 + x + width) / this->width()));
    unsigned physWidth  = static_cast<unsigned>(320.0 * height / this->height());
    unsigned physHeight = static_cast<unsigned>(480.0 * width / this->width());
#endif

    pimpl->queues.back().beginClipping(physX, physY, physWidth, physHeight);
}

void Gosu::Graphics::endClipping()
{
    if (pimpl->queues.size() > 1)
        throw std::logic_error("Clipping is not allowed while creating a macro");
    
    pimpl->queues.back().endClipping();
}

void Gosu::Graphics::beginRecording()
{
    pimpl->queues.resize(pimpl->queues.size() + 1);
}

std::auto_ptr<Gosu::ImageData> Gosu::Graphics::endRecording()
{
    if (pimpl->queues.size() == 1)
        throw std::logic_error("No macro recording in progress that can be captured");
    
    std::auto_ptr<ImageData> result(new Macro(pimpl->queues.back()));
    pimpl->queues.pop_back();
    return result;
}

void Gosu::Graphics::drawLine(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    ZPos z, AlphaMode mode)
{
    DrawOp op;
    
    x1 *= factorX();
    y1 *= factorY();
    x2 *= factorX();
    y2 *= factorY();

    op.mode = mode;
    op.usedVertices = 2;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);

    pimpl->queues.back().addDrawOp(op, z);
}

void Gosu::Graphics::drawTriangle(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    ZPos z, AlphaMode mode)
{
    DrawOp op;
    
    x1 *= factorX();
    y1 *= factorY();
    x2 *= factorX();
    y2 *= factorY();
    x3 *= factorX();
    y3 *= factorY();
    
    op.mode = mode;
    op.usedVertices = 3;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
#ifdef GOSU_IS_IPHONE
    op.usedVertices = 4;
    op.vertices[3] = op.vertices[2];
#endif
    
    pimpl->queues.back().addDrawOp(op, z);
}

void Gosu::Graphics::drawQuad(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    double x4, double y4, Color c4,
    ZPos z, AlphaMode mode)
{
    reorderCoordinatesIfNecessary(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

    DrawOp op;

    x1 *= factorX();
    y1 *= factorY();
    x2 *= factorX();
    y2 *= factorY();
    x3 *= factorX();
    y3 *= factorY();
    x4 *= factorX();
    y4 *= factorY();

    op.mode = mode;
    op.usedVertices = 4;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
// TODO: Should be harmonized
#ifdef GOSU_IS_IPHONE
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[3] = DrawOp::Vertex(x4, y4, c4);
#else
    op.vertices[3] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[2] = DrawOp::Vertex(x4, y4, c4);
#endif

    pimpl->queues.back().addDrawOp(op, z);
}

std::auto_ptr<Gosu::ImageData> Gosu::Graphics::createImage(
    const Bitmap& src, unsigned srcX, unsigned srcY,
    unsigned srcWidth, unsigned srcHeight, unsigned borderFlags)
{
    static const unsigned maxSize = Texture::maxTextureSize();

    // Special case: If the texture is supposed to have hard borders,
    // is quadratic, has a size that is at least 64 pixels but less than 256
    // pixels and a power of two, create a single texture just for this image.
    if ((borderFlags & bfTileable) == bfTileable &&
        srcWidth == srcHeight &&
        (srcWidth & (srcWidth - 1)) == 0 &&
        srcWidth >= 64)
    {
        boost::shared_ptr<Texture> texture(new Texture(srcWidth));
        std::auto_ptr<ImageData> data;
        
        // Use the source bitmap directly if the source area completely covers
        // it.
        if (srcX == 0 && srcWidth == src.width() &&
            srcY == 0 && srcHeight == src.height())
        {
            data = texture->tryAlloc(*this, pimpl->queues, texture,
                    src, 0, 0, src.width(), src.height(), 0);
        }
        else
        {
            Bitmap trimmedSrc;
            trimmedSrc.resize(srcWidth, srcHeight);
            trimmedSrc.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = texture->tryAlloc(*this, pimpl->queues, texture,
                    trimmedSrc, 0, 0, trimmedSrc.width(), trimmedSrc.height(), 0);
        }
        
        if (!data.get())
            throw std::logic_error("Internal texture block allocation error");
        return data;
    }
    
    // Too large to fit on a single texture. 
    if (srcWidth > maxSize - 2 || srcHeight > maxSize - 2)
    {
        Bitmap bmp;
        bmp.resize(srcWidth, srcHeight);
        bmp.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
        std::auto_ptr<ImageData> lidi;
        lidi.reset(new LargeImageData(*this, bmp, maxSize - 2, maxSize - 2, borderFlags));
        return lidi;
    }
    
    Bitmap bmp;
    applyBorderFlags(bmp, src, srcX, srcY, srcWidth, srcHeight, borderFlags);

#if 0
    boost::mutex::scoped_lock lock(pimpl->texMutex);
#endif
    
    // Try to put the bitmap into one of the already allocated textures.
    for (Impl::Textures::iterator i = pimpl->textures.begin(); i != pimpl->textures.end(); ++i)
    {
        boost::shared_ptr<Texture> texture(*i);
        
        std::auto_ptr<ImageData> data;
        data = texture->tryAlloc(*this, pimpl->queues, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
        if (data.get())
            return data;
    }
    
    // All textures are full: Create a new one.
    
    boost::shared_ptr<Texture> texture;
    texture.reset(new Texture(maxSize));
    pimpl->textures.push_back(texture);
    
    std::auto_ptr<ImageData> data;
    data = texture->tryAlloc(*this, pimpl->queues, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");

    return data;
}
