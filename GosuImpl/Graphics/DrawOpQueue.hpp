#ifndef GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP
#define GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP

#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOp.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <map>
#include <vector>

class Gosu::DrawOpQueue
{
    typedef std::vector<DrawOp> DrawOps;
    DrawOps ops;
    typedef std::multimap<ZPos, boost::function<void()> > CodeMap;
    CodeMap code;

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
        ops.swap(other.ops);
        code.swap(other.code);
    }
    
    void scheduleDrawOp(DrawOp op, ZPos z)
    {
        #ifdef GOSU_IS_IPHONE
        // No triangles, no lines supported
        assert (op.usedVertices == 4);
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
        ops.push_back(op);
    }
    
    void scheduleGL(boost::function<void()> customCode, ZPos z)
    {
        code.insert(std::make_pair(z, customCode));
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

    void performDrawOpsAndCode()
    {
        // Allows us to make some assumptions.
        if (ops.empty())
        {
            BOOST_FOREACH (const CodeMap::value_type& fn, code)
                fn.second();
            return;
        }
        
        // Apply Z-Ordering.
        std::stable_sort(ops.begin(), ops.end());
        
        // We will loop: Drawing DrawOps, execute custom code.
        // This means if there is no code, we just draw one batch
        // of DrawOps, so no performance is sacrified.
        DrawOps::const_iterator current = ops.begin(), last = ops.begin();
        CodeMap::const_iterator it = code.begin();
        
        while (true)
        {
            if (it == code.end())
                // Last or only batch of DrawOps:
                // Just draw everything.
                last = ops.end() - 1;
            else
            {
                // There is code waiting:
                // Only draw up to this Z level.
                while (last != ops.end() - 1 && (last + 1)->z < it->first)
                    ++last;
            }
            
            if (current <= last)
            {
                // Draw DrawOps until next code is due
                RenderState renderState;
                while (current < last)
                {
                    DrawOps::const_iterator next = current + 1;
                    current->perform(renderState, &*next);
                    current = next;
                }
                last->perform(renderState, 0);
                ++current;
            }
            
            // Draw next code, or break if there is none
            if (it == code.end())
                break;
            else
            {
                it->second();
                ++it;
            }
        }
    }
    
    void clear()
    {
        // Not sure if Graphics::begin() should implicitly do that.
        //clipRectStack.clear();
        //effectiveRect.reset();
        code.clear();
        ops.clear();
    }
    
    void compileTo(VertexArray& va)
    {
        if (!code.empty())
            throw std::logic_error("Custom code cannot be recorded into a macro");
        
        va.reserve(ops.size());
        std::stable_sort(ops.begin(), ops.end());
        BOOST_FOREACH (const DrawOp& op, ops)
            op.compileTo(va);
    }
};

#endif
