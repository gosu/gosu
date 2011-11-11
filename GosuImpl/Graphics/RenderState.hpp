#ifndef GOSUIMPL_GRAPHICS_RENDERSTATE_HPP
#define GOSUIMPL_GRAPHICS_RENDERSTATE_HPP

#include <GosuImpl/Graphics/Common.hpp>

// Properties that potentially need to be changed between each draw operation.
// This does not include the color or vertex data of the actual quads.
struct Gosu::RenderStateDescriptor
{
    GLuint texName;
    Transform* transform;
    ClipRect clipRect;
    AlphaMode mode;
    
    RenderStateDescriptor()
    : texName(NO_TEXTURE), transform(0), mode(amDefault)
    {
        clipRect.width = NO_CLIPPING;
    }
    
    bool operator==(const RenderStateDescriptor& rhs) const
    {
        return texName == rhs.texName && transform == rhs.transform &&
            clipRect == rhs.clipRect && mode == rhs.mode;
    }
};

// Manages the OpenGL rendering state. It caches the current state, only forwarding the
// changes to OpenGL if the new state is really different.
class Gosu::RenderState : private Gosu::RenderStateDescriptor
{
    // Not copyable
    RenderState(const RenderState&);
    RenderState& operator=(const RenderState&);
    
    void applyTransform()
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        #ifndef GOSU_IS_IPHONE
        glMultMatrixd(&(*transform)[0]);
        #else
        // TODO: Ouch, should always use floats!
        float matrix[16];
        for (int i = 0; i < 16; ++i)
            matrix[i] = (*transform)[i];
        glMultMatrixf(matrix);
        #endif
    }
    
    void applyAlphaMode()
    {
        if (mode == amAdd)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else if (mode == amMultiply)
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
public:
    RenderState()
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    ~RenderState()
    {
        ClipRect noClipping;
        noClipping.width = NO_CLIPPING;
        setClipRect(noClipping);
        setTexName(NO_TEXTURE);
        glPopMatrix();
    }
    
    void setRenderState(const RenderStateDescriptor& rsd)
    {
        setTexName(rsd.texName);
        setTransform(rsd.transform);
        setClipRect(rsd.clipRect);
        setAlphaMode(rsd.mode);
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
    void enforceAfterUntrustedGL()
    {
        // TODO: Actually, we don't have to worry about anything pushed
        // using glPushAttribts because beginGL/endGL will take care of that.
        
        if (texName == NO_TEXTURE)
            glDisable(GL_TEXTURE_2D);
        else
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texName);
        }
        
        applyTransform();
        
        if (clipRect.width == NO_CLIPPING)
            glDisable(GL_SCISSOR_TEST);
        else
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(clipRect.x, clipRect.y, clipRect.width, clipRect.height);
        }

        applyAlphaMode();
    }
};
    
#endif