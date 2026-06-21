#pragma once

#include <Gosu/Transform.hpp>
#include "ClipRectStack.hpp"
#include "GraphicsImpl.hpp"
#include "OpenGLContext.hpp"
#include "Texture.hpp"
#include <optional>

// Properties that potentially need to be changed between each draw operation.
// This does not include the color or vertex data of the actual quads.
struct Gosu::RenderState
{
    std::shared_ptr<Texture> texture;
    const Transform* transform;
    std::optional<Rect> clip_rect;
    BlendMode mode;

    RenderState()
    : transform(0), mode(BM_DEFAULT)
    {
    }

    bool operator==(const RenderState& rhs) const
    {
        return texture == rhs.texture &&
            transform == rhs.transform &&
            clip_rect == rhs.clip_rect &&
            mode == rhs.mode;
    }

    void apply_texture() const
    {
        if (texture) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture->tex_name());
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }
    }

    void apply_alpha_mode() const
    {
        if (mode == BM_ADD) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        }
        else if (mode == BM_MULTIPLY) {
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        }
        else {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void apply_clip_rect() const
    {
        if (clip_rect.has_value()) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(clip_rect->x, clip_rect->y, clip_rect->width, clip_rect->height);
        }
        else {
            glDisable(GL_SCISSOR_TEST);
        }
    }

    // Only used by Macro so far
    #ifndef GOSU_IS_OPENGLES
    void apply() const
    {
        apply_texture();
        // TODO: No inner clip_rect yet - how would this work?!
        apply_alpha_mode();
    }
    #endif
};

// Manages the OpenGL rendering state. It caches the current state, only forwarding the
// changes to OpenGL if the new state is really different.
class Gosu::RenderStateManager : private Gosu::RenderState, private Gosu::Noncopyable
{
    void apply_transform() const
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        #ifndef GOSU_IS_OPENGLES
        glMultMatrixd(transform->matrix.data());
        #else
        // TODO: Ouch, should always use floats!
        GLfloat matrix[16];
        for (int i = 0; i < 16; ++i) {
            matrix[i] = transform->matrix[i];
        }
        glMultMatrixf(matrix);
        #endif
    }

public:
    RenderStateManager()
    {
        apply_alpha_mode();
        // Preserve previous MV matrix
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
    }

    ~RenderStateManager()
    {
        set_clip_rect(std::nullopt);
        set_texture(std::shared_ptr<Texture>());
        // Return to previous MV matrix
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    void set_render_state(const RenderState& rs)
    {
        set_texture(rs.texture);
        set_transform(rs.transform);
        set_clip_rect(rs.clip_rect);
        set_alpha_mode(rs.mode);
    }

    void set_texture(const std::shared_ptr<Texture>& new_texture)
    {
        if (new_texture == texture) {
            return;
        }

        if (new_texture) {
            if (!texture) {
                glEnable(GL_TEXTURE_2D);
            }
            glBindTexture(GL_TEXTURE_2D, new_texture->tex_name());
        }
        else {
            // New texture is NO_TEXTURE, disable texturing.
            glDisable(GL_TEXTURE_2D);
        }
        texture = new_texture;
    }

    void set_transform(const Transform* new_transform)
    {
        if (new_transform == transform) {
            return;
        }

        transform = new_transform;
        apply_transform();
    }

    void set_clip_rect(const std::optional<Rect>& new_clip_rect)
    {
        if (clip_rect == new_clip_rect) {
            return;
        }

        clip_rect = new_clip_rect;
        apply_clip_rect();
    }

    void set_alpha_mode(BlendMode new_mode)
    {
        if (new_mode == mode) {
            return;
        }

        mode = new_mode;
        apply_alpha_mode();
    }

    // The cached values may have been messed with. Reset them again.
    void enforce_after_untrusted_gL() const
    {
        apply_texture();
        apply_transform();
        apply_clip_rect();
        apply_alpha_mode();
    }
};

namespace Gosu
{
    struct VertexArray
    {
        RenderState render_state;
        std::vector<ArrayVertex> vertices;
    };
    typedef std::list<VertexArray> VertexArrays;
}
