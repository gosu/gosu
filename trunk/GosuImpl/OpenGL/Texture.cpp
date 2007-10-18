#include <GosuImpl/OpenGL/Texture.hpp>
#include <GosuImpl/OpenGL/TexChunk.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include <stdexcept>

// TODO: Not threadsafe.
unsigned Gosu::Texture::maxTextureSize()
{
    const static unsigned MIN_SIZE = 256, MAX_SIZE = 512;

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
}

void Gosu::Texture::sync()
{
    if (!shouldSync)
        return;
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, allocator.width(), allocator.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, &pixelData[0]);
    shouldSync = false;
}

Gosu::Texture::Texture(unsigned size)
: allocator(size, size), pixelData(size * size)
{
    glGenTextures(1, &name);
    if (name == static_cast<GLuint>(-1))
        throw std::runtime_error("Couldn't create OpenGL texture");
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    shouldSync = true;
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

#include <Gosu/Timing.hpp> // TODO

std::auto_ptr<Gosu::TexChunk> Gosu::Texture::tryAlloc(Graphics& graphics, DrawOpQueue& queue, boost::shared_ptr<Texture> ptr,
                                                        const Bitmap& bmp, unsigned srcX,
                                                        unsigned srcY, unsigned srcWidth,
                                                        unsigned srcHeight, unsigned padding)
{
    std::auto_ptr<Gosu::TexChunk> result;
    
    boost::optional<BlockAllocator::Block> block = allocator.alloc(srcWidth, srcHeight);
    if (!block)
        return result;
    
    for (unsigned y = 0; y < srcHeight; ++y)
        for (unsigned x = 0; x < srcWidth; ++x)
        {
            boost::uint32_t pixVal = (bmp.getPixel(x, y).argb() & 0x00ffffff) << 8 | bmp.getPixel(x, y).alpha();
            pixVal = bigToNative(pixVal);
            pixelData[(y + block->top) * allocator.width() + (x + block->left)] = pixVal;
        }
    
    shouldSync = true;
    
    result.reset(new TexChunk(graphics, queue, ptr, block->left + padding, block->top + padding,
                              block->width - 2 * padding, block->height - 2 * padding, padding));
    return result;
}

void Gosu::Texture::free(unsigned x, unsigned y)
{
    allocator.free(x, y);
}
