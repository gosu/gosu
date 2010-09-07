#include <Gosu/Graphics.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOp.hpp>
#include <GosuImpl/Graphics/Texture.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <GosuImpl/Graphics/LargeImageData.hpp>
#include <GosuImpl/Graphics/Macro.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Platform.hpp>
#include <boost/foreach.hpp>
#if 0
#include <boost/thread.hpp>
#endif
#include <cmath>
#include <algorithm>
#include <limits>

#ifdef GOSU_IS_IPHONE
#import <UIKit/UIKit.h>
#include <GosuImpl/Orientation.hpp>
#endif

struct Gosu::Graphics::Impl
{
    unsigned virtWidth, virtHeight;
    unsigned physWidth, physHeight;
    bool fullscreen;
    DrawOpQueueStack queues;
    typedef std::vector<boost::shared_ptr<Texture> > Textures;
    Textures textures;
    Transforms currentTransforms;
    Transforms absoluteTransforms;
    
#if 0
    boost::mutex texMutex;
#endif

#ifdef GOSU_IS_IPHONE
    Transform transformForOrientation(Orientation orientation)
    {
        Transform result;
        switch (orientation)
        {
        case orLandscapeLeft:
            result = translate(physWidth, 0);
            result = multiply(rotate(90), result);
            result = multiply(scale(1.0 * physHeight / virtWidth, 1.0 * physWidth / virtHeight), result);
            return result;
        default:
            result = translate(0, physHeight);
            result = multiply(rotate(-90), result);
            result = multiply(scale(1.0 * physHeight / virtWidth, 1.0 * physWidth / virtHeight), result);
            return result;
        }
    } 
    
    Orientation orientation;
    
    Impl()
    : orientation(currentOrientation())
    {
    }
    
    void updateBaseTransform()
    {
        if (orientation != currentOrientation())
        {
            orientation = currentOrientation();
            currentTransforms.front() = transformForOrientation(orientation);
        }
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
    #ifdef GOSU_IS_IPHONE
    std::swap(pimpl->virtWidth, pimpl->virtHeight);
    #endif
    pimpl->fullscreen = fullscreen;

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
    
    // Push one identity matrix as the default transform.
    pimpl->currentTransforms.push_back(scale(1));
    pimpl->absoluteTransforms.push_back(scale(1));
}

Gosu::Graphics::~Graphics()
{
}

unsigned Gosu::Graphics::width() const
{
    double size[2] = { pimpl->virtWidth, pimpl->virtHeight };
    return size[0];
}

unsigned Gosu::Graphics::height() const
{
    double size[2] = { pimpl->virtWidth, pimpl->virtHeight };
    return size[1];
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
    #ifdef GOSU_IS_IPHONE
    pimpl->orientation = static_cast<Gosu::Orientation>(-1);
    #else
    Transform baseTransform;
    baseTransform = scale(1.0 / virtualWidth  * pimpl->physWidth,
                          1.0 / virtualHeight * pimpl->physHeight);
    pimpl->currentTransforms.front() = pimpl->absoluteTransforms.front() = baseTransform;
    #endif
}

bool Gosu::Graphics::begin(Gosu::Color clearWithColor)
{
    // If there is a recording in process, stop it.
    // TODO: Raise exception?
    pimpl->queues.resize(1);
    // Clear leftover clippings.
    pimpl->queues.front().clear();
    
    pimpl->currentTransforms.resize(1);
    #ifdef GOSU_IS_IPHONE
    pimpl->updateBaseTransform();
    #endif
    pimpl->absoluteTransforms = pimpl->currentTransforms;
    
    glClearColor(clearWithColor.red()/255.0,
                 clearWithColor.green()/255.0,
                 clearWithColor.blue()/255.0,
                 clearWithColor.alpha()/255.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    return true;
}

void Gosu::Graphics::end()
{
    flush();

    glFlush();
}

void Gosu::Graphics::flush()
{
    // If there is a recording in process, cancel it.
    pimpl->queues.resize(1);
    
    pimpl->queues.at(0).performDrawOps();
    pimpl->queues.at(0).clear();
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

void Gosu::Graphics::beginClipping(double x, double y, double width, double height)
{
    if (pimpl->queues.size() > 1)
        throw std::logic_error("Clipping not allowed while creating a macro");
    
    // Apply current transformation.
    
    double left = x, right = x + width;
    double top = y, bottom = y + height;
    
    applyTransform(pimpl->absoluteTransforms.back(), left, top);
    applyTransform(pimpl->absoluteTransforms.back(), right, bottom);
    
    int physX = std::min(left, right);
    int physY = std::min(top, bottom);
    int physWidth = std::abs(left - right);
    int physHeight = std::abs(top - bottom);
    
    // Apply OpenGL's counting from the wrong side ;)
    physY = pimpl->physHeight - physY - physHeight;
    
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

namespace
{
    void ensureBackOfList(Gosu::Transforms& list, const Gosu::Transform& transform)
    {
        Gosu::Transforms::iterator oldPosition =
            std::find(list.begin(), list.end(), transform);
        if (oldPosition == list.end())
            list.push_back(transform);
        else
            list.splice(list.end(), list, oldPosition);
    }
}

void Gosu::Graphics::pushTransform(const Gosu::Transform& transform)
{
    pimpl->currentTransforms.push_back(transform);
    Transform result = multiply(transform, pimpl->absoluteTransforms.back());
    ensureBackOfList(pimpl->absoluteTransforms, result);
}

void Gosu::Graphics::popTransform()
{
    pimpl->currentTransforms.pop_back();
    Transform result = scale(1);
    BOOST_REVERSE_FOREACH (const Transform& tf, pimpl->currentTransforms)
        result = multiply(result, tf);
    ensureBackOfList(pimpl->absoluteTransforms, result);
}

void Gosu::Graphics::drawLine(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    ZPos z, AlphaMode mode)
{
    DrawOp op(pimpl->absoluteTransforms.back());
    
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
    DrawOp op(pimpl->absoluteTransforms.back());
        
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

    DrawOp op(pimpl->absoluteTransforms.back());
    
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
            data = texture->tryAlloc(*this, pimpl->absoluteTransforms, pimpl->queues, texture,
                    src, 0, 0, src.width(), src.height(), 0);
        }
        else
        {
            Bitmap trimmedSrc;
            trimmedSrc.resize(srcWidth, srcHeight);
            trimmedSrc.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = texture->tryAlloc(*this, pimpl->absoluteTransforms, pimpl->queues, texture,
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
        data = texture->tryAlloc(*this, pimpl->absoluteTransforms, pimpl->queues, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
        if (data.get())
            return data;
    }
    
    // All textures are full: Create a new one.
    
    boost::shared_ptr<Texture> texture;
    texture.reset(new Texture(maxSize));
    pimpl->textures.push_back(texture);
    
    std::auto_ptr<ImageData> data;
    data = texture->tryAlloc(*this, pimpl->absoluteTransforms, pimpl->queues, texture, bmp, 0, 0, bmp.width(), bmp.height(), 1);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");

    return data;
}
