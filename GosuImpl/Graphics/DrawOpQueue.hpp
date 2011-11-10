#ifndef GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP
#define GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP

#include <Gosu/TR1.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/ClipRectStack.hpp>
#include <GosuImpl/Graphics/DrawOp.hpp>
#include <cassert>
#include <algorithm>
#include <map>
#include <vector>

class Gosu::DrawOpQueue
{
    ClipRectStack clipRectStack;
    
    typedef std::vector<DrawOp> DrawOps;
    DrawOps ops;
    typedef std::multimap<ZPos, std::tr1::function<void()> > CodeMap;
    CodeMap code;
    
public:
    // I really wish I would trust ADL. :|
    void swap(DrawOpQueue& other)
    {
        clipRectStack.swap(other.clipRectStack);
        ops.swap(other.ops);
        code.swap(other.code);
    }
    
    void scheduleDrawOp(DrawOp op, ZPos z)
    {
        #ifdef GOSU_IS_IPHONE
        // No triangles, no lines supported
        assert (op.usedVertices == 4);
        #endif
        
        if (clipRectStack.clippedWorldAway())
            return;
        
        if (const ClipRect* cr = clipRectStack.maybeEffectiveRect())
            op.clipRect = *cr;
        
        op.z = z;
        
        ops.push_back(op);
    }
    
    void scheduleGL(std::tr1::function<void()> customCode, ZPos z)
    {
        code.insert(std::make_pair(z, customCode));
    }
    
    void beginClipping(int x, int y, int width, int height)
    {
        clipRectStack.beginClipping(x, y, width, height);
    }
    
    void endClipping()
    {
        clipRectStack.endClipping();
    }

    void performDrawOpsAndCode()
    {
        // Allows us to make some assumptions.
        if (ops.empty())
        {
            for (CodeMap::iterator it = code.begin(), end = code.end(); it != end; ++it)
                it->second();
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
        for (DrawOps::const_iterator op = ops.begin(), end = ops.end(); op != end; ++op)
            op->compileTo(va);
    }
};

#endif
