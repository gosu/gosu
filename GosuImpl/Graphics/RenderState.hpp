#ifndef GOSUIMPL_GRAPHICS_RENDERSTATE_HPP
#define GOSUIMPL_GRAPHICS_RENDERSTATE_HPP

#include <GosuImpl/Graphics/Common.hpp>

// Properties that potentially need to be changed between each draw operation.
// This does not include the color or vertex data of the actual quads.
struct Gosu::RenderState
{
    GLuint texName;
    Transform* transform;
    ClipRect clipRect;
    AlphaMode mode;
    
    RenderState()
    : texName(NO_TEXTURE), transform(0), mode(amDefault)
    {
        clipRect.width = NO_CLIPPING;
    }
    
    bool operator==(const RenderState& rhs) const
    {
        return texName == rhs.texName && transform == rhs.transform &&
            clipRect == rhs.clipRect && mode == rhs.mode;
    }
    
    void applyTexture() const
    {
        if (texName == NO_TEXTURE)
            glDisable(GL_TEXTURE_2D);
        else
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texName);
        }
    }
    
    void applyAlphaMode() const
    {
        if (mode == amAdd)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else if (mode == amMultiply)
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    void applyClipRect() const
    {
        if (clipRect.width == NO_CLIPPING)
            glDisable(GL_SCISSOR_TEST);
        else
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(clipRect.x, clipRect.y, clipRect.width, clipRect.height);
        }
    }
    
    // Only used by Macro so far
    #ifndef GOSU_IS_IPHONE
    void apply() const
    {
        applyTexture();
        glMultMatrixd(&(*transform)[0]);
        // cliprect from the outside is okay
        applyAlphaMode();
    }
    #endif
};

// Manages the OpenGL rendering state. It caches the current state, only forwarding the
// changes to OpenGL if the new state is really different.
class Gosu::RenderStateManager : private Gosu::RenderState
{
    // Not copyable
    RenderStateManager(const RenderStateManager&);
    RenderStateManager& operator=(const RenderStateManager&);
    
    void applyTransform() const
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPushMatrix();
        
        #ifndef GOSU_IS_IPHONE
        glMultMatrixd(&(*transform)[0]);
        #else
        // TODO: Ouch, should always use floats!
        GLfloat matrix[16];
        for (int i = 0; i < 16; ++i)
            matrix[i] = (*transform)[i];
        glMultMatrixf(matrix);
        #endif
    }
    
public:
    RenderStateManager()
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        applyAlphaMode();
    }
    
    ~RenderStateManager()
    {
        ClipRect noClipping;
        noClipping.width = NO_CLIPPING;
        setClipRect(noClipping);
        setTexName(NO_TEXTURE);
        glPopMatrix();
    }
    
    void setRenderState(const RenderState& rs)
    {
        setTexName(rs.texName);
        setTransform(rs.transform);
        setClipRect(rs.clipRect);
        setAlphaMode(rs.mode);
    }
    
    void setTexName(GLuint newTexName)
    {
        if (newTexName == texName)
            return;
    
        if (newTexName != NO_TEXTURE)
        {
            if (texName == NO_TEXTURE)
                glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, newTexName);
        }
        else if (texName != NO_TEXTURE)
            glDisable(GL_TEXTURE_2D);
        texName = newTexName;
    }
    
    void setTransform(Transform* newTransform)
    {
        if (newTransform == transform)
            return;
        transform = newTransform;
        
        applyTransform();
    }

    void setClipRect(const ClipRect& newClipRect)
    {
        if (newClipRect.width == NO_CLIPPING)
        {
            // Disable clipping
            if (clipRect.width != NO_CLIPPING)
            {
                glDisable(GL_SCISSOR_TEST);
                clipRect.width = NO_CLIPPING;
            }
        }
        else
        {
            // Enable clipping if off
            if (clipRect.width == NO_CLIPPING)
            {
                glEnable(GL_SCISSOR_TEST);
                clipRect = newClipRect;
                glScissor(clipRect.x, clipRect.y, clipRect.width, clipRect.height);
            }
            // Adjust clipping if necessary
            else if (!(clipRect == newClipRect))
            {
                clipRect = newClipRect;
                glScissor(clipRect.x, clipRect.y, clipRect.width, clipRect.height);
            }
        }
    }
    
    void setAlphaMode(AlphaMode newMode)
    {
        if (newMode == mode)
            return;
        mode = newMode;
        applyAlphaMode();
    }
    
    // The cached values may have been messed with. Reset them again.
    void enforceAfterUntrustedGL() const
    {
        // TODO: Actually, we don't have to worry about anything pushed
        // using glPushAttribs because beginGL/endGL will take care of that.
        
        applyTexture();
        applyTransform();
        applyClipRect();
        applyAlphaMode();
    }
};
    
#endif