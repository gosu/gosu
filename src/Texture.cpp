#include "Texture.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Platform.hpp>
#include "TexChunk.hpp"
#include <stdexcept>

namespace Gosu
{
    bool undocumented_retrofication = false;
}

Gosu::Texture::Texture(int width, int height, bool retro)
    : m_bin_packer(width, height),
      m_retro(retro)
{
    ensure_current_context();

    // Create texture name.
    glGenTextures(1, &m_tex_name);
    if (m_tex_name == static_cast<GLuint>(-1)) {
        throw std::runtime_error("Failed to allocate OpenGL texture");
    }

    // Create empty texture.
    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_bin_packer.width(), m_bin_packer.height(), 0,
                 Color::GL_FORMAT, GL_UNSIGNED_BYTE, nullptr);

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

    glDeleteTextures(1, &m_tex_name);
}

std::unique_ptr<Gosu::TexChunk> Gosu::Texture::try_alloc(const Bitmap& bitmap, int padding)
{
    const std::optional<Rect> rect = m_bin_packer.find_rect(bitmap.width(), bitmap.height());

    if (!rect) {
        return nullptr;
    }

    ensure_current_context();

    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    glTexSubImage2D(GL_TEXTURE_2D, 0, rect->x, rect->y, rect->width, rect->height, Color::GL_FORMAT,
                    GL_UNSIGNED_BYTE, bitmap.data());

    // TODO: Can we merge this with TexChunk::insert and have TexChunk update its borders itself?
    const Rect rect_without_padding { rect->x + padding, rect->y + padding,
                                      rect->width - 2 * padding, rect->height - 2 * padding };
    return std::make_unique<TexChunk>(shared_from_this(), rect_without_padding);
}

void Gosu::Texture::block(const Rect& rect)
{
    m_bin_packer.block(rect);
}

void Gosu::Texture::free(const Rect& rect)
{
    m_bin_packer.free(rect);
}

Gosu::Bitmap Gosu::Texture::to_bitmap(const Rect& rect) const
{
#ifdef GOSU_IS_OPENGLES
    // See here for one possible implementation: https://github.com/apitrace/apitrace/issues/70
    // (Could reuse a lot of code from OffScreenTarget)
    throw std::logic_error("Texture::to_bitmap not supported on iOS");
#else
    ensure_current_context();

    Bitmap bitmap(width(), height());
    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    // TODO: There are ways to retrieve only part of a texture, which we should use sooner or later.
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data());

    if (rect != Rect::covering(*this)) {
        Bitmap smaller_bitmap(rect.width, rect.height);
        smaller_bitmap.insert(bitmap, -rect.x, -rect.y);
        bitmap = std::move(smaller_bitmap);
    }

    return bitmap;
#endif
}
