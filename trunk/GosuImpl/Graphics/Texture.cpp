#include <Gosu/Graphics.hpp>
#include <GosuImpl/Graphics/Texture.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include <stdexcept>

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

// TODO: Not threadsafe.
unsigned Gosu::Texture::maxTextureSize()
{
#if defined(GOSU_IS_MAC)
    // Includes the iPhone
    return 1024;
#else
    const static unsigned MIN_SIZE = 256, MAX_SIZE = 1024;

    static unsigned size = 0;
    if (size == 0)
    {
        size = MIN_SIZE / 2;
        GLint width = 1;
        do
        {
            size *= 2;
            glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, size * 2, size * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); 
        } while (width != 0 && size < MAX_SIZE);
    }
    
    return size;
#endif
}

//const unsigned Gosu::MAX_TEXTURE_SIZE = Gosu::Texture::maxTextureSize();

Gosu::Texture::Texture(unsigned size)
: allocator(size, size), num(0)
{
    // Create texture name.
    glGenTextures(1, &name);
    if (name == static_cast<GLuint>(-1))
        throw std::runtime_error("Couldn't create OpenGL texture");
   
    // Create empty texture.
    glBindTexture(GL_TEXTURE_2D, name);
#ifdef GOSU_IS_IPHONE
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, allocator.width(), allocator.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, 4, allocator.width(), allocator.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef GOSU_IS_WIN
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
}

Gosu::Texture::~Texture()
{
    glDeleteTextures(1, &name);
}

unsigned Gosu::Texture::size() const
{
    return allocator.width(); // == height
}

GLuint Gosu::Texture::texName() const
{
    return name;
}

std::auto_ptr<Gosu::TexChunk>
    Gosu::Texture::tryAlloc(Graphics& graphics,
        DrawOpQueueStack& queues, boost::shared_ptr<Texture> ptr,
        const Bitmap& bmp, unsigned srcX,
        unsigned srcY, unsigned srcWidth,
        unsigned srcHeight, unsigned padding)
{
    std::auto_ptr<Gosu::TexChunk> result;
    
    boost::optional<BlockAllocator::Block> block = allocator.alloc(srcWidth, srcHeight);
    if (!block)
        return result;
    
    result.reset(new TexChunk(graphics, queues, ptr, block->left + padding, block->top + padding,
                              block->width - 2 * padding, block->height - 2 * padding, padding));
    
#if defined(__BIG_ENDIAN__)
    std::vector<unsigned> pixelData(srcWidth * srcHeight);
    for (unsigned y = 0; y < srcHeight; ++y)
        for (unsigned x = 0; x < srcWidth; ++x)
        {
            boost::uint32_t pixVal = (bmp.getPixel(x, y).argb() & 0x00ffffff) << 8 | bmp.getPixel(x, y).alpha();
            pixVal = bigToNative(pixVal);
            pixelData[y * srcWidth + x] = pixVal;
        }
    const unsigned* texData = &pixelData[0];
    unsigned format = GL_RGBA;
#elif defined(GOSU_IS_IPHONE)
    std::vector<unsigned> pixelData(srcWidth * srcHeight);
    for (unsigned y = 0; y < srcHeight; ++y)
        for (unsigned x = 0; x < srcWidth; ++x)
            pixelData[y * srcWidth + x] = bmp.getPixel(x, y).abgr();
    const unsigned* texData = &pixelData[0];
    unsigned format = GL_RGBA;
#else
    const unsigned* texData = bmp.data();
    unsigned format = GL_BGRA;
#endif
    
    glBindTexture(GL_TEXTURE_2D, name);
    glTexSubImage2D(GL_TEXTURE_2D, 0, block->left, block->top, block->width, block->height,
                 format, GL_UNSIGNED_BYTE, texData);
    num += 1;
    return result;
}

void Gosu::Texture::free(unsigned x, unsigned y)
{
    allocator.free(x, y);
    num -= 1;
}
