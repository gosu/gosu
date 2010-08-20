#ifndef GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP
#define GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP

#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOp.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <set>
#include <vector>

class Gosu::DrawOpQueue
{
    std::multiset<DrawOp> set;

    struct ClipRect
    {
        int x, y;
        unsigned width, height;
    };
    std::vector<ClipRect> clipRectStack;
    boost::optional<ClipRect> effectiveRect;
    void updateEffectiveRect()
    {
        if (clipRectStack.empty())
            return effectiveRect.reset();

        ClipRect result = { 0, 0, 0x7fffffff, 0x7fffffff };
        BOOST_FOREACH (const ClipRect& rect, clipRectStack)
        {
            int right = std::min<int>(result.x + result.width, rect.x + rect.width);
            int bottom = std::min<int>(result.y + result.height, rect.y + rect.height);
            result.x = std::max<int>(result.x, rect.x);
            result.y = std::max<int>(result.y, rect.y);
            
            if (result.x >= right || result.y >= bottom)
            {
                effectiveRect.reset();
                return;
            }
            
            result.width = right - result.x;
            result.height = bottom - result.y;
        }
        
        int fac = clipRectBaseFactor();
        result.x *= fac, result.y *= fac, result.width *= fac, result.height *= fac;
        
        effectiveRect = result;
    }

public:
    // I really wish I would trust ADL. :|
    void swap(DrawOpQueue& other)
    {
        clipRectStack.swap(other.clipRectStack);
        std::swap(effectiveRect, other.effectiveRect);
        set.swap(other.set);
    }
    
    void addDrawOp(DrawOp op, ZPos z)
    {
        #ifdef GOSU_IS_IPHONE
        // No triangles, no lines supported
        assert(op.usedVertices == 4);
        #endif
        
        if (effectiveRect)
        {
            const ClipRect& rect = *effectiveRect;
            op.clipX = rect.x;
            op.clipY = rect.y;
            op.clipWidth = rect.width;
            op.clipHeight = rect.height;
        }
        else if (!clipRectStack.empty())
            // When we have no effect rect but the stack is not empty, we have clipped
            // the whole world away and don't need to render things.
            return;
        
        op.z = z;
        set.insert(op);
    }
    
    void beginClipping(int x, int y, unsigned width, unsigned height)
    {
        ClipRect rect = { x, y, width, height };
        clipRectStack.push_back(rect);
        updateEffectiveRect();
    }
    
    void endClipping()
    {
        clipRectStack.pop_back();
        updateEffectiveRect();
    }

    void performDrawOps() const
    {
        RenderState current;
        
        std::multiset<DrawOp>::const_iterator last, cur = set.begin(), end = set.end();
        while (cur != end)
        {
            last = cur;
            ++cur;
            last->perform(current, cur == end ? 0 : &*cur);
        }
    }
    
    void clear()
    {
        // Not sure if Graphics::begin() should implicitly do that.
        //clipRectStack.clear();
        //effectiveRect.reset();
        set.clear();
    }
    
    void compileTo(VertexArray& va) const
    {
        va.reserve(set.size());
        BOOST_FOREACH (const DrawOp& op, set)
            op.compileTo(va);
    }
};

#endif
