#include "Texture.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include "OpenGLContext.hpp"
#include "TexChunk.hpp"
#include <stdexcept>

Gosu::Texture::Texture(int width, int height, bool retro)
    : m_bin_packer(width, height),
      m_tex_name(0),
      m_retro(retro)
{
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Gosu::Texture must not be empty");
    }

    const OpenGLContext current_context;

    // Create texture name.
    glGenTextures(1, &m_tex_name);
    // GCOV_EXCL_START: Hard to simulate out-of-texture situations.
    if (m_tex_name == static_cast<GLuint>(-1)) {
        throw std::runtime_error("Failed to allocate OpenGL texture");
    }
    // GCOV_EXCL_END

    // Create empty texture.
    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_bin_packer.width(), m_bin_packer.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    if (retro) {
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
    // Only the first construction of OpenGLContext can throw, but if we get here, then we must have
    // already created a context in the constructor. Therefore, this destructor cannot throw.
    const OpenGLContext current_context;
    glDeleteTextures(1, &m_tex_name);
}

std::unique_ptr<Gosu::TexChunk> Gosu::Texture::try_alloc(const Bitmap& bitmap, int padding)
{
    const std::shared_ptr<const Rect> rect = m_bin_packer.alloc(bitmap.width(), bitmap.height());

    if (!rect) {
        return nullptr;
    }

    insert(bitmap, rect->x, rect->y);

    const Rect rect_without_padding { rect->x + padding, rect->y + padding,
                                      rect->width - 2 * padding, rect->height - 2 * padding };
    return std::make_unique<TexChunk>(shared_from_this(), rect_without_padding, rect);
}

void Gosu::Texture::insert(const Gosu::Bitmap& bitmap, int x, int y)
{
    if (!Rect::covering(*this).contains(Rect { x, y, bitmap.width(), bitmap.height() })) {
        throw std::invalid_argument("Gosu::Texture::insert: Rect exceeds bounds");
    }

    const OpenGLContext current_context;
    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, bitmap.width(), bitmap.height(), GL_RGBA,
                    GL_UNSIGNED_BYTE, bitmap.data());
}

Gosu::Bitmap Gosu::Texture::to_bitmap(const Rect& rect) const
{
    if (!Rect::covering(*this).contains(rect)) {
        throw std::invalid_argument("Gosu::Texture::to_bitmap: Rect exceeds bounds");
    }

#ifdef GOSU_IS_OPENGLES
    // See here for one possible implementation: https://github.com/apitrace/apitrace/issues/70
    // (Could reuse a lot of code from OffScreenTarget)
    throw std::logic_error("Gosu::Texture::to_bitmap not supported in OpenGL ES");
#else
    const OpenGLContext current_context;
    Bitmap bitmap(width(), height());
    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    // This should use glGetTextureSubImage where available: https://stackoverflow.com/a/38148494
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data());

    if (rect != Rect::covering(*this)) {
        Bitmap smaller_bitmap(rect.width, rect.height);
        smaller_bitmap.insert(bitmap, -rect.x, -rect.y);
        bitmap = std::move(smaller_bitmap);
    }

    return bitmap;
#endif
}
