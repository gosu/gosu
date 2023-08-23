#pragma once

#include <Gosu/Utility.hpp>
#include "GraphicsImpl.hpp"
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace Gosu
{
    class ClipRectStack
    {
        std::vector<Rect> m_stack;
        std::optional<Rect> m_effective_rect;

    public:
        void clear();
        void push(const Rect& rect);
        void pop();

        const std::optional<Rect>& effective_rect() const { return m_effective_rect; }
    };
}
