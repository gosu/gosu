#include "ClipRectStack.hpp"
#include <stdexcept>

void Gosu::ClipRectStack::clear()
{
    m_stack.clear();
    m_effective_rect = std::nullopt;
}

void Gosu::ClipRectStack::push(const Rect& rect)
{
    m_stack.push_back(rect);
    if (m_effective_rect) {
        m_effective_rect->clip_to(rect);
    }
    else {
        m_effective_rect = rect;
    }
}

void Gosu::ClipRectStack::pop()
{
    if (m_stack.empty()) {
        throw std::logic_error("ClipRectStack is empty");
    }
    m_stack.pop_back();

    // The clip rect is the intersection of all active clip rects (if any).
    m_effective_rect = std::nullopt;
    for (const auto& rect : m_stack) {
        if (m_effective_rect) {
            m_effective_rect->clip_to(rect);
        }
        else {
            m_effective_rect = rect;
        }
    }
}
