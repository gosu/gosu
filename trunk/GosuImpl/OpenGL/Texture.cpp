#include <GosuImpl/OpenGL/Texture.hpp>
#include <GosuImpl/OpenGL/TexChunk.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include <stdexcept>

void Gosu::OpenGL::Texture::sync()
{
    if (!shouldSync)
        return;
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, allocator.width(), allocator.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, &pixelData[0]);
    shouldSync = false;
}

Gosu::OpenGL::Texture::Texture(unsigned size)
: allocator(size, size), pixelData(size * size)
{
    glGenTextures(1, &name);
    if (name == static_cast<GLuint>(-1))
        throw std::runtime_error("Couldn't create OpenGL texture");
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    shouldSync = true;
}

Gosu::OpenGL::Texture::~Texture()
{
    glDeleteTextures(1, &name);
}

unsigned Gosu::OpenGL::Texture::size() const
{
    return allocator.width(); // == height
}

GLuint Gosu::OpenGL::Texture::texName() const
{
    return name;
}

std::auto_ptr<Gosu::OpenGL::TexChunk> Gosu::OpenGL::Texture::tryAlloc(Graphics& graphics, BlitQueue<Blit>& queue, boost::shared_ptr<Texture> ptr,
                                                        const Bitmap& bmp, unsigned srcX,
                                                        unsigned srcY, unsigned srcWidth,
                                                        unsigned srcHeight, unsigned padding)
{
    std::auto_ptr<Gosu::OpenGL::TexChunk> result;
    
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

void Gosu::OpenGL::Texture::free(unsigned x, unsigned y)
{
    allocator.free(x, y);
}
