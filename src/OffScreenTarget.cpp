#include "OffScreenTarget.hpp"
#include "Texture.hpp"
#include <Gosu/Image.hpp>
#include <SDL.h>
using namespace std;

Gosu::OffScreenTarget::OffScreenTarget(int width, int height)
{
    // Create a new texture as the our rendering target.
    texture = make_shared<Texture>(width, height, false);
    // Mark the full texture as blocked for our TexChunk.
    texture->block(0, 0, width, height);
    
    // Besides the texture, also create a renderbuffer for depth information. Gosu doesn't use this,
    // but custom OpenGL could might.
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    // Now tie everything together.
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture->tex_name(), 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
}

Gosu::OffScreenTarget::~OffScreenTarget()
{
    glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &framebuffer);
}

Gosu::Image Gosu::OffScreenTarget::render(const std::function<void ()>& f)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) throw runtime_error("Incomplete framebuffer");
    
    f();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unique_ptr<ImageData> tex_chunk(new TexChunk(texture, 0, 0, texture->width(), texture->height(), 0));
    return Image(move(tex_chunk));
}
