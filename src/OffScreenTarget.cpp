#include "OffScreenTarget.hpp"
#include "Texture.hpp"
#include <Gosu/Image.hpp>
#include <Gosu/Platform.hpp>
#ifndef GOSU_IS_IPHONE
#include <SDL.h>
#endif
using namespace std;

#ifdef GOSU_IS_OPENGLES
    #define GOSU_LOAD_GL_EXT(fn, type) \
        static auto fn = fn ## OES;

    #define GOSU_GL_CONST(name) \
        name ## _OES

    #define GOSU_GL_DEPTH_COMPONENT \
        GL_DEPTH_COMPONENT16_OES
#else
    #define GOSU_LOAD_GL_EXT(fn, type) \
        static auto fn = (type) SDL_GL_GetProcAddress(#fn); \
        if (!fn) throw runtime_error("Unable to load " #fn);

    #define GOSU_GL_CONST(name) \
        name

    #define GOSU_GL_DEPTH_COMPONENT \
        GL_DEPTH_COMPONENT
#endif

Gosu::OffScreenTarget::OffScreenTarget(int width, int height, unsigned image_flags)
{
#ifndef GOSU_IS_IPHONE
    if (!SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object")) {
        throw runtime_error("Missing GL_EXT_framebuffer_object extension");
    }
#endif
    
    // Create a new texture that will be our rendering target.
    texture = make_shared<Texture>(width, height, image_flags & IF_RETRO);
    // Mark the full texture as blocked for our TexChunk.
    texture->block(0, 0, width, height);
    
    // Besides the texture, also create a renderbuffer for depth information.
    // Gosu doesn't use this, but custom OpenGL code could might.
    GOSU_LOAD_GL_EXT(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
    glGenRenderbuffers(1, &renderbuffer);
    
    GOSU_LOAD_GL_EXT(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
    glBindRenderbuffer(GOSU_GL_CONST(GL_RENDERBUFFER), renderbuffer);
    
    GOSU_LOAD_GL_EXT(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
    glRenderbufferStorage(GOSU_GL_CONST(GL_RENDERBUFFER), GOSU_GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GOSU_GL_CONST(GL_RENDERBUFFER), 0);
    
    // Now tie everything together.
    GOSU_LOAD_GL_EXT(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
    glGenFramebuffers(1, &framebuffer);
    
    GOSU_LOAD_GL_EXT(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
    glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), framebuffer);
    
    GOSU_LOAD_GL_EXT(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
    glFramebufferTexture2D(GOSU_GL_CONST(GL_FRAMEBUFFER), GOSU_GL_CONST(GL_COLOR_ATTACHMENT0),
                           GL_TEXTURE_2D, texture->tex_name(), 0);
    
    GOSU_LOAD_GL_EXT(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
    glFramebufferRenderbuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), GOSU_GL_CONST(GL_DEPTH_ATTACHMENT),
                              GOSU_GL_CONST(GL_RENDERBUFFER), renderbuffer);
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
    glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), framebuffer);
    
    GOSU_LOAD_GL_EXT(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
    GLenum status = glCheckFramebufferStatus(GOSU_GL_CONST(GL_FRAMEBUFFER));
    if (status != GOSU_GL_CONST(GL_FRAMEBUFFER_COMPLETE)) throw runtime_error("Incomplete framebuffer");
    
    f();
    glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), 0);

    unique_ptr<ImageData> tex_chunk(new TexChunk(texture, 0, 0, texture->width(), texture->height(), 0));
    return Image(move(tex_chunk));
}
