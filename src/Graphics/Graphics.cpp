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
#include <cmath>
#include <algorithm>
#include <limits>

namespace Gosu
{
    namespace
    {
        Graphics* currentGraphicsPointer = 0;
        
        Graphics& currentGraphics()
        {
            if (currentGraphicsPointer == 0)
                throw std::logic_error("Gosu::Graphics can only be drawn to while rendering");
            
            return *currentGraphicsPointer;
        }
        
        typedef std::vector<std::tr1::shared_ptr<Texture> > Textures;
        Textures textures;
    }
}

struct Gosu::Graphics::Impl
{
    unsigned virtWidth, virtHeight;
    unsigned physWidth, physHeight;
    double blackWidth, blackHeight;
    bool fullscreen;
    DrawOpQueueStack queues;
};

Gosu::Graphics::Graphics(unsigned physWidth, unsigned physHeight, bool fullscreen)
: pimpl(new Impl)
{
    pimpl->physWidth  = physWidth;
    pimpl->physHeight = physHeight;
    pimpl->virtWidth  = physWidth;
    pimpl->virtHeight = physHeight;
    pimpl->fullscreen = fullscreen;
    pimpl->blackWidth = 0;
    pimpl->blackHeight = 0;
    
    // TODO: Should be merged into RenderState and removed from Graphics.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, physWidth, physHeight);
    #ifdef GOSU_IS_OPENGLES
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
    if (currentGraphicsPointer == this)
        currentGraphicsPointer = 0;
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

void Gosu::Graphics::setResolution(unsigned virtualWidth, unsigned virtualHeight,
    double horizontalBlackBarWidth, double verticalBlackBarHeight)
{
    if (virtualWidth == 0 || virtualHeight == 0)
        throw std::invalid_argument("Invalid virtual resolution.");
    
    pimpl->virtWidth = virtualWidth;
    pimpl->virtHeight = virtualHeight;
    pimpl->blackWidth = horizontalBlackBarWidth;
    pimpl->blackHeight = verticalBlackBarHeight;

    double scaleX = 1.0 * pimpl->physWidth / virtualWidth;
    double scaleY = 1.0 * pimpl->physHeight / virtualHeight;
    double scaleFactor = std::min(scaleX, scaleY);

    Transform scaleTransform = scale(scaleFactor);
    Transform translateTransform = translate(pimpl->blackWidth, pimpl->blackHeight);
    Transform baseTransform = concat(translateTransform, scaleTransform);
    
    pimpl->queues.front().setBaseTransform(baseTransform);
}

bool Gosu::Graphics::begin(Gosu::Color clearWithColor)
{
    // If recording is in process, cancel it.
    assert (pimpl->queues.size() == 1);
    pimpl->queues.resize(1);
    // Clear leftover transforms, clip rects etc.
    pimpl->queues.front().reset();
    
    glClearColor(clearWithColor.red() / 255.f, clearWithColor.green() / 255.f,
        clearWithColor.blue() / 255.f, clearWithColor.alpha() / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (currentGraphicsPointer != 0)
        throw std::logic_error("Cannot nest calls to Gosu::Graphics::begin()");
    
    currentGraphicsPointer = this;
    
    return true;
}

void Gosu::Graphics::end()
{
    // If recording is in process, cancel it.
    pimpl->queues.resize(1);
    
    flush();

    if (pimpl->blackHeight || pimpl->blackWidth) {
        if (pimpl->blackHeight) {
            drawQuad(0, -pimpl->blackHeight, Color::BLACK,
                     width(), -pimpl->blackHeight, Color::BLACK,
                     0, 0, Color::BLACK,
                     width(), 0, Color::BLACK, 0);
            drawQuad(0, height(), Color::BLACK,
                     width(), height(), Color::BLACK,
                     0, height() + pimpl->blackHeight, Color::BLACK,
                     width(), height() + pimpl->blackHeight, Color::BLACK, 0);
        }
        else if (pimpl->blackWidth) {
            drawQuad(-pimpl->blackWidth, 0, Color::BLACK,
                     0, 0, Color::BLACK,
                     -pimpl->blackWidth, height(), Color::BLACK,
                     0, height(), Color::BLACK, 0);
            drawQuad(width(), 0, Color::BLACK,
                     width() + pimpl->blackWidth, 0, Color::BLACK,
                     width(), height(), Color::BLACK,
                     width() + pimpl->blackWidth, height(), Color::BLACK, 0);
        }
        flush();
    }
    
    glFlush();

    currentGraphicsPointer = 0;
}

void Gosu::Graphics::flush()
{
    Graphics& cg = currentGraphics();
    
    if (cg.pimpl->queues.size() != 1)
        throw std::logic_error("Flushing to screen is not allowed while creating a macro");
    
    cg.pimpl->queues.front().performDrawOpsAndCode();
    cg.pimpl->queues.front().clearQueue();
}

void Gosu::Graphics::beginGL()
{
    Graphics& cg = currentGraphics();
    
    if (cg.pimpl->queues.size() > 1)
        throw std::logic_error("Custom OpenGL is not allowed while creating a macro");
    
#ifdef GOSU_IS_OPENGLES
    throw std::logic_error("Custom OpenGL ES is not supported yet");
#else
    flush();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_BLEND);
    while (glGetError() != GL_NO_ERROR);
#endif
}

void Gosu::Graphics::endGL()
{
#ifdef GOSU_IS_OPENGLES
    throw std::logic_error("Custom OpenGL ES is not supported yet");
#else
    Graphics& cg = currentGraphics();
    
    glPopAttrib();

    // Restore matrices.
    // TODO: Should be merged into RenderState and removed from Graphics.

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, cg.pimpl->physWidth, cg.pimpl->physHeight);
    glOrtho(0, cg.pimpl->physWidth, cg.pimpl->physHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
#endif
}

#ifdef GOSU_IS_OPENGLES
void Gosu::Graphics::scheduleGL(const std::tr1::function<void()>& functor, Gosu::ZPos z)
{
    throw std::logic_error("Custom OpenGL ES is not supported yet");
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
    Graphics& cg = currentGraphics();
    
    cg.pimpl->queues.back().scheduleGL(RunGLFunctor(cg, functor), z);
}
#endif

void Gosu::Graphics::beginClipping(double x, double y, double width, double height)
{
    Graphics& cg = currentGraphics();
    
    if (cg.pimpl->queues.size() > 1)
        throw std::logic_error("Clipping is not allowed while creating a macro yet");
    
    cg.pimpl->queues.back().beginClipping(x, y, width, height, cg.pimpl->physHeight);
}

void Gosu::Graphics::endClipping()
{
    Graphics& cg = currentGraphics();
    
    cg.pimpl->queues.back().endClipping();
}

void Gosu::Graphics::beginRecording()
{
    Graphics& cg = currentGraphics();
    
    cg.pimpl->queues.resize(cg.pimpl->queues.size() + 1);
}

GOSU_UNIQUE_PTR<Gosu::ImageData> Gosu::Graphics::endRecording(int width, int height)
{
    Graphics& cg = currentGraphics();
    
    if (cg.pimpl->queues.size() == 1)
        throw std::logic_error("No macro recording in progress that can be captured");
    
    GOSU_UNIQUE_PTR<ImageData> result(new Macro(cg, cg.pimpl->queues.back(), width, height));
    cg.pimpl->queues.pop_back();
    return result;
}

void Gosu::Graphics::pushTransform(const Gosu::Transform& transform)
{
    Graphics& cg = currentGraphics();
    
    cg.pimpl->queues.back().pushTransform(transform);
}

void Gosu::Graphics::popTransform()
{
    Graphics& cg = currentGraphics();
    
    cg.pimpl->queues.back().popTransform();
}

void Gosu::Graphics::drawLine(double x1, double y1, Color c1,
    double x2, double y2, Color c2, ZPos z, AlphaMode mode)
{
    Graphics& cg = currentGraphics();
    
    DrawOp op;
    op.renderState.mode = mode;
    op.verticesOrBlockIndex = 2;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.z = z;
    cg.pimpl->queues.back().scheduleDrawOp(op);
}

void Gosu::Graphics::drawTriangle(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    ZPos z, AlphaMode mode)
{
    Graphics& cg = currentGraphics();
    
    DrawOp op;
    op.renderState.mode = mode;
    op.verticesOrBlockIndex = 3;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
#ifdef GOSU_IS_OPENGLES
    op.verticesOrBlockIndex = 4;
    op.vertices[3] = op.vertices[2];
#endif
    op.z = z;
    cg.pimpl->queues.back().scheduleDrawOp(op);
}

void Gosu::Graphics::drawQuad(double x1, double y1, Color c1,
    double x2, double y2, Color c2, double x3, double y3, Color c3,
    double x4, double y4, Color c4, ZPos z, AlphaMode mode)
{
    Graphics& cg = currentGraphics();
    
    reorderCoordinatesIfNecessary(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

    DrawOp op;
    op.renderState.mode = mode;
    op.verticesOrBlockIndex = 4;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
// TODO: Should be harmonized
#ifdef GOSU_IS_OPENGLES
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[3] = DrawOp::Vertex(x4, y4, c4);
#else
    op.vertices[3] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[2] = DrawOp::Vertex(x4, y4, c4);
#endif
    op.z = z;
    cg.pimpl->queues.back().scheduleDrawOp(op);
}

void Gosu::Graphics::scheduleDrawOp(const Gosu::DrawOp &op)
{
    Graphics& cg = currentGraphics();
    
    cg.pimpl->queues.back().scheduleDrawOp(op);
}

GOSU_UNIQUE_PTR<Gosu::ImageData> Gosu::Graphics::createImage(
    const Bitmap& src, unsigned srcX, unsigned srcY,
    unsigned srcWidth, unsigned srcHeight, unsigned flags)
{
    static const unsigned maxSize = MAX_TEXTURE_SIZE;
    
    // Backwards compatibility: This used to be 'bool tileable'.
    if (flags == 1)
        flags = ifTileable;

    // Special case: If the texture is supposed to have hard borders, is
    // quadratic, has a size that is at least 64 pixels but no more than maxSize
    // pixels and a power of two, create a single texture just for this image.
    if ((flags & ifTileable) == ifTileable &&
        srcWidth == srcHeight &&
        (srcWidth & (srcWidth - 1)) == 0 &&
        srcWidth >= 64 && srcWidth <= maxSize)
    {
        std::tr1::shared_ptr<Texture> texture(new Texture(srcWidth));
        GOSU_UNIQUE_PTR<ImageData> data;
        
        // Use the source bitmap directly if the source area completely covers
        // it.
        if (srcX == 0 && srcWidth == src.width() &&
            srcY == 0 && srcHeight == src.height())
        {
            data = texture->tryAlloc(texture, src, 0);
        }
        else
        {
            Bitmap bmp(srcWidth, srcHeight);
            bmp.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
            data = texture->tryAlloc(texture, bmp, 0);
        }
        
        if (!data.get())
            throw std::logic_error("Internal texture block allocation error");
        return GOSU_MOVE_UNIQUE_PTR(data);
    }
    
    // Too large to fit on a single texture. 
    if (srcWidth > maxSize - 2 || srcHeight > maxSize - 2)
    {
        Bitmap bmp(srcWidth, srcHeight);
        bmp.insert(src, 0, 0, srcX, srcY, srcWidth, srcHeight);
        GOSU_UNIQUE_PTR<ImageData> lidi;
        lidi.reset(new LargeImageData(bmp, maxSize - 2, maxSize - 2, flags));
        return GOSU_MOVE_UNIQUE_PTR(lidi);
    }
    
    Bitmap bmp;
    applyBorderFlags(bmp, src, srcX, srcY, srcWidth, srcHeight, flags);

    // Try to put the bitmap into one of the already allocated textures.
    for (Textures::iterator i = textures.begin(); i != textures.end(); ++i)
    {
        std::tr1::shared_ptr<Texture> texture(*i);
        
        GOSU_UNIQUE_PTR<ImageData> data;
        data = texture->tryAlloc(texture, bmp, 1);
        if (data.get())
            return GOSU_MOVE_UNIQUE_PTR(data);
    }
    
    // All textures are full: Create a new one.
    
    std::tr1::shared_ptr<Texture> texture;
    texture.reset(new Texture(maxSize));
    textures.push_back(texture);
    
    GOSU_UNIQUE_PTR<ImageData> data;
    data = texture->tryAlloc(texture, bmp, 1);
    if (!data.get())
        throw std::logic_error("Internal texture block allocation error");

    return GOSU_MOVE_UNIQUE_PTR(data);
}
