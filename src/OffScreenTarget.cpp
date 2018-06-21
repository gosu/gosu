#include "OffScreenTarget.hpp"
#include "Texture.hpp"
#include <Gosu/Image.hpp>
#include <Gosu/Platform.hpp>
#include <SDL.h>
using namespace std;

#define GOSU_LOAD_GL_EXT(fn, type) \
    static auto fn = (type) SDL_GL_GetProcAddress(#fn); \
    if (!fn) throw runtime_error("Unable to load " #fn);

Gosu::OffScreenTarget::OffScreenTarget(int width, int height)
{
    if (!SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object")) {
        throw runtime_error("Missing GL_EXT_framebuffer_object extension");
    }
    
    // Create a new texture that will be our rendering target.
    texture = make_shared<Texture>(width, height, false);
    // Mark the full texture as blocked for our TexChunk.
    texture->block(0, 0, width, height);
    
    // Besides the texture, also create a renderbuffer for depth information.
    // Gosu doesn't use this, but custom OpenGL code could might.
    GOSU_LOAD_GL_EXT(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
    glGenRenderbuffers(1, &renderbuffer);
    
    GOSU_LOAD_GL_EXT(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    GOSU_LOAD_GL_EXT(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    // Now tie everything together.
    GOSU_LOAD_GL_EXT(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
    glGenFramebuffers(1, &framebuffer);
    GOSU_LOAD_GL_EXT(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GOSU_LOAD_GL_EXT(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture->tex_name(), 0);
    GOSU_LOAD_GL_EXT(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
}

Gosu::OffScreenTarget::~OffScreenTarget()
{
    try {
        GOSU_LOAD_GL_EXT(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC);
        glDeleteRenderbuffers(1, &renderbuffer);
        GOSU_LOAD_GL_EXT(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC);
        glDeleteFramebuffers(1, &framebuffer);
    } catch (...) {
        // If we can't load these functions, just accept the resource leak.
    }
}

Gosu::Image Gosu::OffScreenTarget::render(const std::function<void ()>& f)
{
    GOSU_LOAD_GL_EXT(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    GOSU_LOAD_GL_EXT(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) throw runtime_error("Incomplete framebuffer");
    
    f();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unique_ptr<ImageData> tex_chunk(new TexChunk(texture, 0, 0, texture->width(), texture->height(), 0));
    return Image(move(tex_chunk));
}
