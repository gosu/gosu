#pragma once

#include <Gosu/Utility.hpp>
#include <cstdint>
#include <functional>
#include <memory>

namespace Gosu
{
    class Image;
    class Texture;

    class OffScreenTarget : private Noncopyable
    {
        std::shared_ptr<Texture> m_texture;
        std::uint32_t r_renderbuffer;
        std::uint32_t m_framebuffer;

    public:
        OffScreenTarget(int width, int height, unsigned image_flags);
        ~OffScreenTarget();

        Gosu::Image render(const std::function<void ()>& f);
    };
}
