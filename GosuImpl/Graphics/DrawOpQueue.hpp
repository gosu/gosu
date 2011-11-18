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
    typedef std::vector<std::tr1::function<void()> > GLBlocks;
    GLBlocks glBlocks;
    
public:
    // I really wish I would trust ADL. :|
    void swap(DrawOpQueue& other)
    {
        clipRectStack.swap(other.clipRectStack);
        ops.swap(other.ops);
        glBlocks.swap(other.glBlocks);
    }
    
    void scheduleDrawOp(DrawOp op)
    {
        if (clipRectStack.clippedWorldAway())
            return;
        
        #ifdef GOSU_IS_IPHONE
        // No triangles, no lines supported
        assert (op.verticesOrBlockIndex == 4);
        #endif
        
        if (const ClipRect* cr = clipRectStack.maybeEffectiveRect())
            op.renderState.clipRect = *cr;
        ops.push_back(op);
    }
    
    void scheduleGL(Transform& transform, std::tr1::function<void()> glBlock, ZPos z)
    {
        // TODO: Document this case: Clipped-away GL blocks are *not* being run.
        if (clipRectStack.clippedWorldAway())
            return;
        
        int complementOfBlockIndex = ~(int)glBlocks.size();
        glBlocks.push_back(glBlock);
        
        DrawOp op;
        op.renderState.transform = &transform;
        op.verticesOrBlockIndex = complementOfBlockIndex;
        if (const ClipRect* cr = clipRectStack.maybeEffectiveRect())
            op.renderState.clipRect = *cr;
        op.z = z;
        ops.push_back(op);
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
        // Apply Z-Ordering.
        std::stable_sort(ops.begin(), ops.end());
        
        RenderStateManager manager;
        #ifdef GOSU_IS_IPHONE
        if (ops.empty())
            return;
        
        DrawOps::const_iterator current = ops.begin(), last = ops.end() - 1;
        for (; current != last; ++current)
            current->perform(&*(current + 1));
        last->perform(renderState, 0);
        #else
        for (DrawOps::const_iterator current = ops.begin(), last = ops.end();
            current != last; ++current)
        {
            manager.setRenderState(current->renderState);
            if (current->verticesOrBlockIndex >= 0)
            {
                // Normal DrawOp, no GL code
                current->perform(0); // next unused on desktop
            }
            else
            {
                // GL code
                int blockIndex = ~current->verticesOrBlockIndex;
                assert (blockIndex >= 0);
                assert (blockIndex < glBlocks.size());
                glBlocks[blockIndex]();
                manager.enforceAfterUntrustedGL();
            }
        }
        #endif
    }
    
    void clear()
    {
        glBlocks.clear();
        ops.clear();
    }
    
    void compileTo(VertexArrays& vas)
    {
        if (!glBlocks.empty())
            throw std::logic_error("Custom code cannot be recorded into a macro");
        
        std::stable_sort(ops.begin(), ops.end());
        for (DrawOps::const_iterator op = ops.begin(), end = ops.end(); op != end; ++op)
            op->compileTo(vas);
    }
};

#endif
