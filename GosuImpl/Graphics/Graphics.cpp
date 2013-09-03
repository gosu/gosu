#include <Gosu/Graphics.hpp>
#include "Common.hpp"
#include "DrawOp.hpp"
#include "Texture.hpp"
#include "TexChunk.hpp"
#include "LargeImageData.hpp"
#include "Macro.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Platform.hpp>
#if 0
#include <thread>
#endif
#include <cmath>
#include <algorithm>
#include <limits>

#ifdef GOSU_IS_IPHONE
#include "../Orientation.hpp"
#endif

struct Gosu::Graphics::Impl
{
    unsigned virtWidth, virtHeight;
    unsigned physWidth, physHeight;
    bool fullscreen;
    DrawOpQueueStack queues;
    typedef std::vector<std::tr1::shared_ptr<Texture> > Textures;
    Textures textures;
    
#if 0
    std::mutex texMutex;
#endif

#ifdef GOSU_IS_IPHONE
    Transform transformForOrientation(Orientation orientation)
    {
        Transform result;
        switch (orientation)
        {
        case orLandscapeLeft:
            result = translate(0, 0);
            result = multiply(scale(1.0 * physWidth / virtWidth, 1.0 * physHeight / virtHeight), result);
            return result;
        default:
            result = translate(0, 0);
            result = multiply(scale(1.0 * physWidth / virtWidth, 1.0 * physHeight / virtHeight), result);
            return result;
        }
    }
    
    void updateBaseTransform()
    {
        queues.front().setBaseTransform(transformForOrientation(currentOrientation()));
    }
#endif
};

Gosu::Graphics::Graphics(unsigned physWidth, unsigned physHeight, bool fullscreen)
: pimpl(new Impl)
{
    pimpl->physWidth  = physWidth;
    pimpl->physHeight = physHeight;
    pimpl->virtWidth  = physWidth;
    pimpl->virtHeight = physHeight;
    pimpl->fullscreen = fullscreen;
    
    // Should be merged into RenderState altogether.
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, physWidth, physHeight);
    #ifdef GOSU_IS_IPHONE
    glOrthof(0, physWidth, physHeight, 0, -1, 1);
    #else
    glOrtho(0, physWidth, physHeight, 0, -1, 1);
    #endif
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
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

void Gosu::Graphics::setResolution(unsigned virtualWidth, unsigned virtualHeight)
{
    if (virtualWidth == 0 || virtualHeight == 0)
        throw std::invalid_argument("Invalid virtual resolution.");
    
    pimpl->virtWidth = virtualWidth, pimpl->virtHeight = virtualHeight;
    #ifndef GOSU_IS_IPHONE
    // on the iPhone, updateCurrentTransform will handle this (yuck)
    Transform baseTransform = scale(1.0 / virtualWidth  * pimpl->physWidth,
                                    1.0 / virtualHeight * pimpl->physHeight);
    pimpl->queues.front().setBaseTransform(baseTransform);
    #endif
}

bool Gosu::Graphics::begin(Gosu::Color clearWithColor)
{
    // If recording is in process, cancel it.
    assert (pimpl->queues.size() == 1);
    pimpl->queues.resize(1);
    // Clear leftover transforms, clip rects etc.
    pimpl->queues.front().reset();
    
    #ifdef GOSU_IS_IPHONE
    pimpl->updateBaseTransform();
    #endif
    glClearColor(clearWithColor.red() / 255.f, clearWithColor.green() / 255.f,
        clearWithColor.blue() / 255.f, clearWithColor.alpha() / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    return true;
}

void Gosu::Graphics::end()
{
    // If recording is in process, cancel it.
    assert (pimpl->queues.size() == 1);
    pimpl->queues.resize(1);
    
    flush();
    
    glFlush();
}

void Gosu::Graphics::flush()
{
    if (pimpl->queues.size() != 1)
        throw std::logic_error("Flushing to screen is not allowed while creating a macro");
    
    pimpl->queues.front().performDrawOpsAndCode();
    pimpl->queues.front().clearQueue();
}

void Gosu::Graphics::beginGL()
{
    if (pimpl->queues.size() > 1)
        throw std::logic_error("Custom OpenGL is not allowed while creating a macro");
    
#ifdef GOSU_IS_IPHONE
    throw std::logic_error("Custom OpenGL is unsupported on the iPhone");
#else
    flush();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_BLEND);
    while (glGetError() != GL_NO_ERROR);
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
    glOrtho(0, pimpl->physWidth, pimpl->physHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
#endif
}

#ifdef GOSU_IS_IPHONE
void Gosu::Graphics::scheduleGL(const std::tr1::function<void()>& functor, Gosu::ZPos z)
{
    throw std::logic_error("Custom OpenGL is unsupported on the iPhone");
}
#else
namespace Gosu
{
    struct RunGLFunctor
    {
        Graphics& graphics;
        std::tr1::function<void()> functor;
        
        RunGLFunctor(Graphics& graphics, const std::tr1::function<void()>& functor)
        : graphics(graphics), functor(functor)
        {
        }
        
        void operator()() const
        {
            // Inlined beginGL() to avoid flushing.
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glDisable(GL_BLEND);
            while (glGetError() != GL_NO_ERROR);
            
            functor();
            
            // Does not have to be inlined.
            graphics.endGL();            
        }
    };
}

void Gosu::Graphics::scheduleGL(const std::tr1::function<void()>& functor, Gosu::ZPos z)
{
    pimpl->queues.back().scheduleGL(RunGLFunctor(*this, functor), z);
}
#endif

void Gosu::Graphics::beginClipping(double x, double y, double width, double height)
{
    if (pimpl->queues.size() > 1)
        throw std::logic_error("Clipping is not allowed while creating a macro yet");
    
    pimpl->queues.back().beginClipping(x, y, width, height, pimpl->physHeight);
}

void Gosu::Graphics::endClipping()
{
    pimpl->queues.back().endClipping();
}

void Gosu::Graphics::beginRecording()
{
    pimpl->queues.resize(pimpl->queues.size() + 1);
}

std::auto_ptr<Gosu::ImageData> Gosu::Graphics::endRecording(int width, int height)
{
    if (pimpl->queues.size() == 1)
        throw std::logic_error("No macro recording in progress that can be captured");
    
    std::auto_ptr<ImageData> result(new Macro(*this, pimpl->queues.back(), width, height));
    pimpl->queues.pop_back();
    return result;
}

void Gosu::Graphics::pushTransform(const Gosu::Transform& transform)
{
    pimpl->queues.back().pushTransform(transform);
}

void Gosu::Graphics::popTransform()
{
    pimpl->queues.back().popTransform();
}

void Gosu::Graphics::drawLine(double x1, double y1, Color c1,
    double x2, double y2, Color c2, ZPos z, AlphaMode mode)
{
    DrawOp op;
    op.renderState.mode = mode;
    op.verticesOrBlockIndex = 2;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.z = z;
    pimpl->queues.back().scheduleDrawOp(op);
}

void Gosu::Graphics::drawTriangle(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    ZPos z, AlphaMode mode)
{
    DrawOp op;
    op.renderState.mode = mode;
    op.verticesOrBlockIndex = 3;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
#ifdef GOSU_IS_IPHONE
    op.verticesOrBlockIndex = 4;
    op.vertices[3] = op.vertices[2];
#endif
    op.z = z;
    pimpl->queues.back().scheduleDrawOp(op);
}

void Gosu::Graphics::drawQuad(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    double x4, double y4, Color c4, ZPos z, AlphaMode mode)
{
    reorderCoordinatesIfNecessary(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

    DrawOp op;
    op.renderState.mode = mode;
    op.verticesOrBlockIndex = 4;
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
    op.z = z;
    pimpl->queues.back().scheduleDrawOp(op);
}

std::auto_ptr<Gosu::ImageData> Gosu::Graphics::createImage(
    const Bitmap& src, unsigned srcX, unsigned srcY,
    unsigned srcWidth, unsigned srcHeight, unsigned borderFlags)
{
    static const unsigned maxSize = MAX_TEXTURE_SIZE;

    // Special case: If the texture is supposed to have hard borders,
    // is quadratic, has a size that is at least 64 pixels but less than 256
    // pixels and a power of two, create a single texture just for this image.
    if ((borderFlags & bfTileable) == bfTileable &&
        srcWidth == srcHeight &&
        (srcWidth & (srcWidth - 1)) == 0 &&
        srcWidth >= 64)
    {
        std::tr1::shared_ptr<Texture> texture(new Texture(srcWidth));
        std::auto_ptr<ImageData> data;
        
        // Use the source bitmap directly if the source area completely covers
        // it.
        if (srcX == 0 && srcWidth == src.width() &&
            srcY == 0 && srcHeight == src.height())
        {
            data = texture->tryAlloc(*this, pimpl->queues, texture, src, 0);
        }
        else
        {
            Bitmap trimmedSrc;
            trimmedSrc.resize(srcWidth, srcHeight);
            trimmedSrc.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = texture->tryAlloc(*this, pimpl->queues, texture, trimmedSrc, 0);
        }
        
        if (!data.get())
            throw std::logic_error("Internal texture block allocation error");
        return data;
    }
    
    // Too large to fit on a single texture. 
    if (srcWidth > maxSize - 2 || srcHeight > maxSize - 2)
    {
        Bitmap bmp(srcWidth, srcHeight);
        bmp.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
        std::auto_ptr<ImageData> lidi;
        lidi.reset(new LargeImageData(*this, bmp, maxSize - 2, maxSize - 2, borderFlags));
        return lidi;
    }
    
    Bitmap bmp;
    applyBorderFlags(bmp, src, srcX, srcY, srcWidth, srcHeight, borderFlags);

#if 0
    std::mutex::scoped_lock lock(pimpl->texMutex);
#endif
    
    // Try to put the bitmap into one of the already allocated textures.
    for (Impl::Textures::iterator i = pimpl->textures.begin(); i != pimpl->textures.end(); ++i)
    {
        std::tr1::shared_ptr<Texture> texture(*i);
        
        std::auto_ptr<ImageData> data;
        data = texture->tryAlloc(*this, pimpl->queues, texture, bmp, 1);
        if (data.get())
            return data;
    }
    
    // All textures are full: Create a new one.
    
    std::tr1::shared_ptr<Texture> texture;
    texture.reset(new Texture(maxSize));
    pimpl->textures.push_back(texture);
    
    std::auto_ptr<ImageData> data;
    data = texture->tryAlloc(*this, pimpl->queues, texture, bmp, 1);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");

    return data;
}
