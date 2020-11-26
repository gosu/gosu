#include "Texture.hpp"
#include "TexChunk.hpp"
#include "Log.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Platform.hpp>
#include <stdexcept>
using namespace std;

namespace Gosu
{
    bool undocumented_retrofication = false;
}

Gosu::Texture::Texture(unsigned width, unsigned height, bool retro)
: allocator_(width, height), retro_(retro)
{
    log("Allocating a new texture of size %dx%d (retro=%d)", width, height, (int) retro);

    ensure_current_context();
    
    // Create texture name.
    glGenTextures(1, &tex_name_);
    if (tex_name_ == static_cast<GLuint>(-1)) throw runtime_error("Couldn't create OpenGL texture");

    // Create empty texture.
    glBindTexture(GL_TEXTURE_2D, tex_name_);
#ifdef GOSU_IS_OPENGLES
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, allocator_.width(), allocator_.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, 4, allocator_.width(), allocator_.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
#endif
    
    if (retro || undocumented_retrofication) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else {
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
    ensure_current_context();
    
    glDeleteTextures(1, &tex_name_);
}

unsigned Gosu::Texture::width() const
{
    return allocator_.width();
}

unsigned Gosu::Texture::height() const
{
    return allocator_.height();
}

GLuint Gosu::Texture::tex_name() const
{
    return tex_name_;
}

bool Gosu::Texture::retro() const
{
    return retro_;
}

unique_ptr<Gosu::TexChunk> Gosu::Texture::try_alloc(const Bitmap& bmp, unsigned padding)
{
    BlockAllocator::Block block;
    
    if (!allocator_.alloc(bmp.width(), bmp.height(), block)) return nullptr;
    
    unique_ptr<TexChunk> result(new TexChunk(shared_from_this(),
                                             block.left   + padding,
                                             block.top    + padding,
                                             block.width  - 2 * padding,
                                             block.height - 2 * padding,
                                             padding));
    
    ensure_current_context();
    
    glBindTexture(GL_TEXTURE_2D, tex_name_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, block.left, block.top, block.width, block.height,
                    Color::GL_FORMAT, GL_UNSIGNED_BYTE, bmp.data());

    return result;
}

void Gosu::Texture::block(unsigned x, unsigned y, unsigned width, unsigned height)
{
    allocator_.block(x, y, width, height);
}

void Gosu::Texture::free(unsigned x, unsigned y, unsigned width, unsigned height)
{
    allocator_.free(x, y, width, height);
}

Gosu::Bitmap Gosu::Texture::to_bitmap(unsigned x, unsigned y, unsigned width, unsigned height) const
{
#ifdef GOSU_IS_OPENGLES
    // See here for one possible implementation: https://github.com/apitrace/apitrace/issues/70
    // (Could reuse a lot of code from OffScreenTarget)
    throw logic_error("Texture::to_bitmap not supported on iOS");
#else
    ensure_current_context();
    
    Bitmap full_texture(this->width(), this->height());
    glBindTexture(GL_TEXTURE_2D, tex_name());
    // TODO: There are ways to retrieve only part of a texture, which we should use sooner or later.
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, full_texture.data());
    Bitmap bitmap(width, height);
    bitmap.insert(-int(x), -int(y), full_texture);
    
    return bitmap;
#endif
}
