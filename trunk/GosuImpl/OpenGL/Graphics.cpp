#include <Gosu/Graphics.hpp>
#include <GosuImpl/Graphics/Graphics.hpp>
#include <GosuImpl/OpenGL/OpenGL.hpp>
#include <GosuImpl/OpenGL/Blit.hpp>
#include <GosuImpl/OpenGL/Texture.hpp>
#include <GosuImpl/OpenGL/TexChunk.hpp>
#include <GosuImpl/Graphics/LargeImageData.hpp>
#include <Gosu/Bitmap.hpp>

struct Gosu::Graphics::Impl
{
    unsigned width, height;
    double factorX, factorY;
    bool fullscreen;
    BlitQueue<OpenGL::Blit> queue;
    typedef std::vector<boost::weak_ptr<OpenGL::Texture> > Textures;
    Textures textures;
};

boost::optional<Gosu::DisplayMode> Gosu::DisplayMode::find(unsigned width,
    unsigned height, bool fullscreen)  // FIXME!
{
    DisplayMode mode;
    mode.width = width;
    mode.height = height;
    mode.fullscreen = fullscreen;
    return mode;
}

Gosu::Graphics::Graphics(const DisplayMode& mode)
: pimpl(new Impl)
{
    pimpl->width = mode.width;
    pimpl->height = mode.height;
    pimpl->factorX = pimpl->factorY = 1.0;
    pimpl->fullscreen = mode.fullscreen;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, pimpl->width, pimpl->height);
    glOrtho(0, pimpl->width, pimpl->height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

    return true; // FIXME!
}

void Gosu::Graphics::end()
{
    for (unsigned i = 0; i < pimpl->textures.size(); ++i)
        if (!pimpl->textures[i].expired())
            boost::shared_ptr<OpenGL::Texture>(pimpl->textures[i])->sync();
    pimpl->queue.performBlits();
//    glXSwapBuffers(dpy, window);
    glFlush();
}

void Gosu::Graphics::drawLine(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    ZPos z, AlphaMode mode)
{
    OpenGL::Blit newBlit;
    
    x1 *= factorX();
    y1 *= factorY();
    x2 *= factorX();
    y2 *= factorY();

    newBlit.mode = mode;
    newBlit.usedVertices = 2;
    newBlit.vertices[0] = OpenGL::Blit::Vertex(x1, y1, c1);
    newBlit.vertices[1] = OpenGL::Blit::Vertex(x2, y2, c2);

    pimpl->queue.addBlit(newBlit, z);
}

void Gosu::Graphics::drawTriangle(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    ZPos z, AlphaMode mode)
{
    OpenGL::Blit newBlit;

    x1 *= factorX();
    y1 *= factorY();
    x2 *= factorX();
    y2 *= factorY();
    x3 *= factorX();
    y3 *= factorY();

    newBlit.mode = mode;
    newBlit.usedVertices = 3;
    newBlit.vertices[0] = OpenGL::Blit::Vertex(x1, y1, c1);
    newBlit.vertices[1] = OpenGL::Blit::Vertex(x2, y2, c2);
    newBlit.vertices[2] = OpenGL::Blit::Vertex(x3, y3, c3);

    pimpl->queue.addBlit(newBlit, z);
}

void Gosu::Graphics::drawQuad(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    double x4, double y4, Color c4,
    ZPos z, AlphaMode mode)
{
    OpenGL::Blit newBlit;

    x1 *= factorX();
    y1 *= factorY();
    x2 *= factorX();
    y2 *= factorY();
    x3 *= factorX();
    y3 *= factorY();
    x4 *= factorX();
    y4 *= factorY();

    newBlit.mode = mode;
    newBlit.usedVertices = 4;
    newBlit.vertices[0] = OpenGL::Blit::Vertex(x1, y1, c1);
    newBlit.vertices[1] = OpenGL::Blit::Vertex(x2, y2, c2);
    newBlit.vertices[3] = OpenGL::Blit::Vertex(x3, y3, c3);
    newBlit.vertices[2] = OpenGL::Blit::Vertex(x4, y4, c4);

    pimpl->queue.addBlit(newBlit, z);
}

std::auto_ptr<Gosu::ImageData> Gosu::Graphics::createImage(
    const Bitmap& src, unsigned srcX, unsigned srcY,
    unsigned srcWidth, unsigned srcHeight, unsigned borderFlags)
{
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
            data = ImageDataImpl::tryToCreate(*this, src, texture, 0, 0, 0, 0);
        }
        else
        {
            Bitmap trimmedSrc;
            trimmedSrc.resize(srcWidth, srcHeight);
            trimmedSrc.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = ImageDataImpl::tryToCreate(*this, trimmedSrc, texture, 0, 0,
                                              0, 0);
        }
        
        if (!data.get())
            throw std::logic_error("Internal texture block allocation error");
        return data;
    }*/
    
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
    applyBorderFlags(bmp, src, srcX, srcY, srcWidth, srcHeight, borderFlags);
    
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
        
        boost::shared_ptr<OpenGL::Texture> texture(*i);
        
        std::auto_ptr<ImageData> data;
        data = texture->tryAlloc(*this, pimpl->queue, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
        if (data.get())
            return data;
    }
    
    // All textures are full: Create a new one.
    
    boost::shared_ptr<OpenGL::Texture> texture;
    texture.reset(new OpenGL::Texture(256));
    pimpl->textures.push_back(texture);
    
    std::auto_ptr<ImageData> data;
    data = texture->tryAlloc(*this, pimpl->queue, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");
    return data;
}
