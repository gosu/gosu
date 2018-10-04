#pragma once

#include "GraphicsImpl.hpp"

namespace Gosu
{
    class OffScreenTarget
    {
        std::shared_ptr<Texture> texture;
        GLuint renderbuffer;
        GLuint framebuffer;
        
        OffScreenTarget(const OffScreenTarget& other) = delete;
        OffScreenTarget& operator=(const OffScreenTarget& other) = delete;
        OffScreenTarget(OffScreenTarget&& other) = delete;
        OffScreenTarget& operator=(OffScreenTarget&& other) = delete;
        
    public:
        OffScreenTarget(int width, int height, unsigned image_flags);
        ~OffScreenTarget();
        Gosu::Image render(const std::function<void ()>& f);
    };
}
