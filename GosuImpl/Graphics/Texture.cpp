#include <Gosu/Graphics.hpp>
#include "Texture.hpp"
#include "TexChunk.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include <stdexcept>

namespace Gosu
{
    bool undocumentedRetrofication = false;
}

Gosu::Texture::Texture(unsigned size)
: allocator(size, size)
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
    
    if (undocumentedRetrofication)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
#ifdef GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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

GOSU_UNIQUE_PTR<Gosu::TexChunk>
    Gosu::Texture::tryAlloc(Graphics& graphics, DrawOpQueueStack& queues,
        std::tr1::shared_ptr<Texture> ptr, const Bitmap& bmp, unsigned padding)
{
    GOSU_UNIQUE_PTR<Gosu::TexChunk> result;
    
    BlockAllocator::Block block;
    if (!allocator.alloc(bmp.width(), bmp.height(), block))
        return result;
    
    result.reset(new TexChunk(graphics, queues, ptr, block.left + padding, block.top + padding,
                              block.width - 2 * padding, block.height - 2 * padding, padding));
    
    glBindTexture(GL_TEXTURE_2D, name);
    glTexSubImage2D(GL_TEXTURE_2D, 0, block.left, block.top, block.width, block.height,
                 Color::GL_FORMAT, GL_UNSIGNED_BYTE, bmp.data());

    return GOSU_MOVE_UNIQUE_PTR(result);
}

void Gosu::Texture::block(unsigned x, unsigned y, unsigned width, unsigned height)
{
    allocator.block(x, y, width, height);
}

void Gosu::Texture::free(unsigned x, unsigned y, unsigned width, unsigned height)
{
    allocator.free(x, y, width, height);
}

Gosu::Bitmap Gosu::Texture::toBitmap(unsigned x, unsigned y, unsigned width, unsigned height) const
{
#ifdef GOSU_IS_IPHONE
    throw std::logic_error("Texture::toBitmap not supported on iOS");
#else
    Gosu::Bitmap fullTexture(size(), size());
    glBindTexture(GL_TEXTURE_2D, name);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, fullTexture.data());
    Gosu::Bitmap bitmap(width, height);
    bitmap.insert(fullTexture, -int(x), -int(y));
    
    return bitmap;
#endif
}
