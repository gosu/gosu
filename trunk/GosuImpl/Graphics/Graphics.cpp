#include <Gosu/Graphics.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOp.hpp>
#include <GosuImpl/Graphics/Texture.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <GosuImpl/Graphics/LargeImageData.hpp>
#include <Gosu/Bitmap.hpp>
#include <boost/thread.hpp>

struct Gosu::Graphics::Impl
{
    unsigned width, height;
    bool fullscreen;
    double factorX, factorY;
    DrawOpQueue queue;
    typedef std::vector<boost::shared_ptr<Texture> > Textures;
    Textures textures;
    boost::mutex texMutex;
};

Gosu::Graphics::Graphics(unsigned width, unsigned height, bool fullscreen)
: pimpl(new Impl)
{
    pimpl->width = width;
    pimpl->height = height;
    pimpl->fullscreen = fullscreen;
    pimpl->factorX = pimpl->factorY = 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, pimpl->width, pimpl->height);
    glOrtho(0, pimpl->width, pimpl->height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
}

Gosu::Graphics::~Graphics()
{
}

unsigned Gosu::Graphics::width() const
{
    return pimpl->width;
}

unsigned Gosu::Graphics::height() const
{
    return pimpl->height;
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

double Gosu::Graphics::virtualWidth() const
{
    return width() / factorX();
}

double Gosu::Graphics::virtualHeight() const
{
    return height() / factorY();
}

void Gosu::Graphics::setVirtualResolution(double virtualWidth,
                                          double virtualHeight)
{
    if (virtualWidth == 0 || virtualHeight == 0)
        throw std::invalid_argument("Invalid virtual resolution.");
    
    pimpl->factorX = width() / virtualWidth;
    pimpl->factorY = height() / virtualHeight;
}

bool Gosu::Graphics::begin(Gosu::Color clearWithColor)
{
    glClearColor(clearWithColor.red()/255.0,
                 clearWithColor.green()/255.0,
                 clearWithColor.blue()/255.0,
                 clearWithColor.alpha()/255.0);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
}

void Gosu::Graphics::end()
{
    pimpl->queue.performDrawOps();
    glFlush();
}

void Gosu::Graphics::beginGL()
{
    pimpl->queue.performDrawOps();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_BLEND);
    // TODO: completely restore default state
}

void Gosu::Graphics::endGL()
{
    glPopAttrib();

    // Restore matrices.
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, pimpl->width, pimpl->height);
    glOrtho(0, pimpl->width, pimpl->height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
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

    pimpl->queue.addDrawOp(op, z);
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

    pimpl->queue.addDrawOp(op, z);
}

void Gosu::Graphics::drawQuad(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    double x4, double y4, Color c4,
    ZPos z, AlphaMode mode)
{
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
    op.vertices[3] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[2] = DrawOp::Vertex(x4, y4, c4);

    pimpl->queue.addDrawOp(op, z);
}

std::auto_ptr<Gosu::ImageData> Gosu::Graphics::createImage(
    const Bitmap& src, unsigned srcX, unsigned srcY,
    unsigned srcWidth, unsigned srcHeight, unsigned borderFlags)
{
    static const unsigned maxSize = Texture::maxTextureSize();

    // Special case: If the texture is supposed to have hard borders,
    // is quadratic, has a size that is at least 64 pixels but less than 256
    // pixels and a power of two, create a single texture just for this image.
    /*if ((borderFlags & bfHard) == bfHard &&
        srcWidth == srcHeight &&
        srcWidth >= 64 && srcWidth <= 256 &&
        (srcWidth & (srcWidth - 1)) == 0)
    {
        boost::shared_ptr<Texture> texture(new Texture(srcWidth));
        std::auto_ptr<ImageData> data;
        
        // Use the source bitmap directly if the source area completely covers
        // it.
        if (srcX == 0 && srcWidth == src.width() &&
            srcY == 0 && srcHeight == src.height())
        {
            data = texture->tryAlloc(*this, pimpl->queue, texture,
                    src, 0, 0, src.width(), src.height(), 0);
        }
        else
        {
            Bitmap trimmedSrc;
            trimmedSrc.resize(srcWidth, srcHeight);
            trimmedSrc.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = texture->tryAlloc(*this, pimpl->queue, texture,
                    trimmedSrc, 0, 0, trimmedSrc.width(), trimmedSrc.height(), 0);
        }
        
        if (!data.get())
            throw std::logic_error("Internal texture block allocation error");
        return data;
    }*/
    
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
    
    boost::mutex::scoped_lock lock(pimpl->texMutex);
    
    // Try to put the bitmap into one of the already allocated textures.
    for (Impl::Textures::iterator i = pimpl->textures.begin(); i != pimpl->textures.end(); ++i)
    {
        boost::shared_ptr<Texture> texture(*i);
        
        std::auto_ptr<ImageData> data;
        data = texture->tryAlloc(*this, pimpl->queue, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
        if (data.get())
            return data;
    }
    
    // All textures are full: Create a new one.
    
    boost::shared_ptr<Texture> texture;
    texture.reset(new Texture(maxSize));
    pimpl->textures.push_back(texture);
    
    std::auto_ptr<ImageData> data;
    data = texture->tryAlloc(*this, pimpl->queue, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");

    return data;
}
