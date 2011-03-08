#ifndef GOSUIMPL_GRAPHICS_RENDERSTATE_HPP
#define GOSUIMPL_GRAPHICS_RENDERSTATE_HPP

#include <GosuImpl/Graphics/Common.hpp>

class Gosu::RenderState
{
    GLuint texName;
    Transform* transform;
    unsigned clipX, clipY, clipWidth, clipHeight;
    AlphaMode mode;
    
public:
    RenderState()
    :   texName(NO_TEXTURE), transform(0), clipWidth(NO_CLIPPING), mode(amDefault)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    ~RenderState()
    {
        setClipRect(NO_CLIPPING, NO_CLIPPING, NO_CLIPPING, NO_CLIPPING);
        setTexName(NO_TEXTURE);
        glPopMatrix();
    }
    
    void setTransform(Transform* newTransform)
    {
        if (newTransform == transform)
            return;
            
        glPopMatrix();
        glPushMatrix();
        #ifndef GOSU_IS_IPHONE
        glMultMatrixd(newTransform->data());
        #else
        boost::array<float, 16> matrix;
        matrix = *newTransform;
        glMultMatrixf(matrix.data());
        #endif
        transform = newTransform;
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
    
    void setClipRect(unsigned newClipX, unsigned newClipY,
        unsigned newClipWidth, unsigned newClipHeight)
    {
        if (newClipWidth == NO_CLIPPING)
        {
            // Disable clipping
            if (clipWidth != NO_CLIPPING)
            {
                glDisable(GL_SCISSOR_TEST);
                clipWidth = NO_CLIPPING;
            }
        }
        else
        {
            // Enable clipping if off
            if (clipWidth == NO_CLIPPING)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(clipX = newClipX, clipY = newClipY,
                    clipWidth = newClipWidth, clipHeight = newClipHeight);
            }
            // Adjust clipping if necessary
            else if (clipX != newClipX || clipY != newClipY ||
                clipWidth != newClipWidth || clipHeight != newClipHeight)
            {
                glScissor(clipX = newClipX, clipY = newClipY,
                    clipWidth = newClipWidth, clipHeight = newClipHeight);
            }
        }
    }
    
    void setAlphaMode(AlphaMode newMode)
    {
        if (newMode == mode)
            return;
        mode = newMode;
        if (mode == amAdditive)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else if (mode == amMultiply)
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};
    
#endif