#include "OffScreenTarget.hpp"
#include <Gosu/Image.hpp>
#include <Gosu/Platform.hpp>
#include "OpenGLContext.hpp"
#include "Texture.hpp"
#ifndef GOSU_IS_IPHONE
#include <SDL.h> // for SDL_GL_GetProcAddress
#endif

#ifdef GOSU_IS_OPENGLES
#define GOSU_LOAD_GL_EXT(fn, type) static auto fn = fn##OES
#define GOSU_GL_CONST(name) name##_OES
#define GOSU_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT16_OES
#else
#define GOSU_LOAD_GL_EXT(fn, type)                                                                 \
    static auto fn = reinterpret_cast<type>(SDL_GL_GetProcAddress(#fn));                           \
    if ((fn) == nullptr) {                                                                         \
        throw std::runtime_error("Unable to load " #fn);                                           \
    }
#define GOSU_GL_CONST(name) name
#define GOSU_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT
#endif

Gosu::OffScreenTarget::OffScreenTarget(int width, int height, unsigned image_flags)
    : r_renderbuffer(static_cast<GLuint>(-1)),
      m_framebuffer(static_cast<GLuint>(-1))
{
#ifndef GOSU_IS_IPHONE
    // GCOV_EXCL_START
    if (!SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object")) {
        throw std::runtime_error("Missing GL_EXT_framebuffer_object extension");
    }
    // GCOV_EXCL_end
#endif

    // Create a new texture that will be our rendering target.
    m_texture = std::make_shared<Texture>(width, height, image_flags & IF_RETRO);

    // Besides the texture, also create a renderbuffer for depth information.
    // Gosu doesn't use this, but custom OpenGL code might.
    GOSU_LOAD_GL_EXT(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
    glGenRenderbuffers(1, &r_renderbuffer);

    GOSU_LOAD_GL_EXT(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
    glBindRenderbuffer(GOSU_GL_CONST(GL_RENDERBUFFER), r_renderbuffer);

    GOSU_LOAD_GL_EXT(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
    glRenderbufferStorage(GOSU_GL_CONST(GL_RENDERBUFFER), GOSU_GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GOSU_GL_CONST(GL_RENDERBUFFER), 0);

    // Now tie everything together.
    GOSU_LOAD_GL_EXT(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
    glGenFramebuffers(1, &m_framebuffer);

    GOSU_LOAD_GL_EXT(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
    glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), m_framebuffer);

    GOSU_LOAD_GL_EXT(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
    glFramebufferTexture2D(GOSU_GL_CONST(GL_FRAMEBUFFER), GOSU_GL_CONST(GL_COLOR_ATTACHMENT0),
                           GL_TEXTURE_2D, m_texture->tex_name(), 0);

    GOSU_LOAD_GL_EXT(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
    glFramebufferRenderbuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), GOSU_GL_CONST(GL_DEPTH_ATTACHMENT),
                              GOSU_GL_CONST(GL_RENDERBUFFER), r_renderbuffer);
}

Gosu::OffScreenTarget::~OffScreenTarget()
{
    try {
        GOSU_LOAD_GL_EXT(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC);
        glDeleteRenderbuffers(1, &r_renderbuffer);

        GOSU_LOAD_GL_EXT(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC);
        glDeleteFramebuffers(1, &m_framebuffer);
        // GCOV_EXCL_START
    } catch (...) {
        // If we can't load these functions, just accept the resource leak.
    }
    // GCOV_EXCL_END
}

Gosu::Image Gosu::OffScreenTarget::render(const std::function<void ()>& f)
{
    GOSU_LOAD_GL_EXT(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
    glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), m_framebuffer);

    GOSU_LOAD_GL_EXT(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
    GLenum status = glCheckFramebufferStatus(GOSU_GL_CONST(GL_FRAMEBUFFER));
    if (status != GOSU_GL_CONST(GL_FRAMEBUFFER_COMPLETE)) {
        throw std::runtime_error("Incomplete framebuffer");
    }

    try {
        f();

    } catch (...) {
        glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), 0);
        throw;
    }
    glBindFramebuffer(GOSU_GL_CONST(GL_FRAMEBUFFER), 0);

    return Image(std::make_unique<TexChunk>(m_texture, Rect::covering(*m_texture), nullptr));
}
