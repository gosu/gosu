#ifndef GOSUIMPL_GRAPHICS_CLIPRECTSTACK_HPP
#define GOSUIMPL_GRAPHICS_CLIPRECTSTACK_HPP

#include "Common.hpp"
#include <cassert>
#include <vector>

class Gosu::ClipRectStack
{
    std::vector<ClipRect> stack;
    bool hasEffectiveRect; // is effectiveRect valid?
    ClipRect effectiveRect;
    
    void updateEffectiveRect()
    {
        // Nothing to do, no clipping in place.
        if (stack.empty())
        {
            hasEffectiveRect = false;
            return;
        }
        
        ClipRect result = { 0.0, 0.0, 1e10, 1e10 };
        for (int i = 0, end = stack.size(); i < end; ++i)
        {
            const ClipRect& rect = stack[i];
            int resultRight = std::min(result.x + result.width, rect.x + rect.width);
            int resultBottom = std::min(result.y + result.height, rect.y + rect.height);
            result.x = std::max(result.x, rect.x);
            result.y = std::max(result.y, rect.y);
            
            if (result.x >= resultRight || result.y >= resultBottom)
            {
                // We have clipped the world away!
                hasEffectiveRect = false;
                return;
            }
            
            result.width = resultRight - result.x;
            result.height = resultBottom - result.y;
        }
        
        // On the iPhone, we may have to multiple everything by 2 for retina displays.
        // TODO: This should be handled by a global transform.
        int fac = clipRectBaseFactor();
        result.x *= fac, result.y *= fac, result.width *= fac, result.height *= fac;
        
        // Normal clipping.
        effectiveRect = result;
        hasEffectiveRect = true;
    }
    
public:    
    ClipRectStack()
    : hasEffectiveRect(false)
    {
    }
    
    void clear()
    {
        stack.clear();
        hasEffectiveRect = false;
    }
    
    void beginClipping(double x, double y, double width, double height)
    {
        ClipRect rect = { x, y, width, height };
        stack.push_back(rect);
        updateEffectiveRect();
    }
    
    void endClipping()
    {
        assert (!stack.empty());
        stack.pop_back();
        updateEffectiveRect();
    }
    
    const ClipRect* maybeEffectiveRect() const
    {
        return hasEffectiveRect ? &effectiveRect : 0;
    }
    
    bool clippedWorldAway() const
    {
        // When we have no effective rect but the stack is not empty, we have clipped
        // the whole world away and don't need to render things.
        return !hasEffectiveRect && !stack.empty();
    }
};

#endif
