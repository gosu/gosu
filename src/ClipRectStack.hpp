#pragma once

#include "GraphicsImpl.hpp"
#include <cassert>
#include <vector>

class Gosu::ClipRectStack
{
    std::vector<ClipRect> stack;
    bool has_effective_rect; // is effective_rect valid?
    ClipRect effective_rect;
    
    void update_effective_rect()
    {
        // Nothing to do, no clipping in place.
        if (stack.empty()) {
            has_effective_rect = false;
            return;
        }
        
        ClipRect result = { 0.0, 0.0, 1e10, 1e10 };
        for (std::size_t i = 0, end = stack.size(); i < end; ++i) {
            const ClipRect& rect = stack[i];
            int result_right     = std::min(result.x + result.width, rect.x + rect.width);
            int result_bottom    = std::min(result.y + result.height, rect.y + rect.height);
            result.x             = std::max(result.x, rect.x);
            result.y             = std::max(result.y, rect.y);
            
            if (result.x >= result_right || result.y >= result_bottom) {
                // We have clipped the world away!
                has_effective_rect = false;
                return;
            }
            
            result.width  = result_right - result.x;
            result.height = result_bottom - result.y;
        }
        
        // On the iPhone, we may have to multiply everything by 2 for Retina displays.
        // TODO: Doesn't this affect Retina Macs as well?
        // TODO: This should be handled by a global transform.
        int fac = clip_rect_base_factor();
        result.x *= fac;
        result.y *= fac;
        result.width *= fac;
        result.height *= fac;
        
        // Normal clipping.
        effective_rect     = result;
        has_effective_rect = true;
    }
    
public:
    ClipRectStack()
    : has_effective_rect(false)
    {
    }
    
    void clear()
    {
        stack.clear();
        has_effective_rect = false;
    }
    
    void begin_clipping(double x, double y, double width, double height)
    {
        ClipRect rect = { x, y, width, height };
        stack.push_back(rect);
        update_effective_rect();
    }
    
    void end_clipping()
    {
        assert (!stack.empty());
        stack.pop_back();
        update_effective_rect();
    }
    
    const ClipRect* maybe_effective_rect() const
    {
        return has_effective_rect ? &effective_rect : 0;
    }
    
    bool clipped_world_away() const
    {
        // When we have no effective rect but the stack is not empty, we have clipped
        // the whole world away and don't need to render things.
        return !has_effective_rect && !stack.empty();
    }
};
