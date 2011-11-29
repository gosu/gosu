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
    Transforms individualTransforms;
    Transforms absoluteTransforms;
    ClipRectStack clipRectStack;
    
    typedef std::vector<DrawOp> DrawOps;
    DrawOps ops;
    typedef std::vector<std::tr1::function<void()> > GLBlocks;
    GLBlocks glBlocks;
    
    void makeCurrentTransform(const Transform& transform)
    {
        Transforms::iterator oldPosition =
            std::find(absoluteTransforms.begin(), absoluteTransforms.end(), transform);
        if (oldPosition == absoluteTransforms.end())
            absoluteTransforms.push_back(transform);
        else
            absoluteTransforms.splice(absoluteTransforms.end(), absoluteTransforms, oldPosition);
    }
    
    Transform& currentTransform()
    {
        return absoluteTransforms.back();
    }
    
public:
    DrawOpQueue()
    {
        // Every queue has a base transform that is always the current transform.
        // This keeps the code a bit more uniform, and allows the window to
        // set a base transform in the main rendering queue.
        individualTransforms.push_back(scale(1));
        absoluteTransforms.push_back(scale(1));
    }
    
    void scheduleDrawOp(DrawOp op)
    {
        if (clipRectStack.clippedWorldAway())
            return;
        
        #ifdef GOSU_IS_IPHONE
        // No triangles, no lines supported
        assert (op.verticesOrBlockIndex == 4);
        #endif
        
        op.renderState.transform = &currentTransform();
        if (const ClipRect* cr = clipRectStack.maybeEffectiveRect())
            op.renderState.clipRect = *cr;
        ops.push_back(op);
    }
    
    void scheduleGL(std::tr1::function<void()> glBlock, ZPos z)
    {
        // TODO: Document this case: Clipped-away GL blocks are *not* being run.
        if (clipRectStack.clippedWorldAway())
            return;
        
        int complementOfBlockIndex = ~(int)glBlocks.size();
        glBlocks.push_back(glBlock);
        
        DrawOp op;
        op.verticesOrBlockIndex = complementOfBlockIndex;
        op.renderState.transform = &currentTransform();
        if (const ClipRect* cr = clipRectStack.maybeEffectiveRect())
            op.renderState.clipRect = *cr;
        op.z = z;
        ops.push_back(op);
    }
    
    void beginClipping(int x, int y, int width, int height, int screenHeight)
    {
        // Apply current transformation.
        
        double left = x, right = x + width;
        double top = y, bottom = y + height;
        
        applyTransform(currentTransform(), left, top);
        applyTransform(currentTransform(), right, bottom);
        
        int physX = std::min(left, right);
        int physY = std::min(top, bottom);
        int physWidth = std::abs(int(left - right));
        int physHeight = std::abs(int(top - bottom));
        
        // Adjust for OpenGL having the wrong idea of where y=0 is.
        // TODO: This should really happen *right before* setting up
        // the glScissor.
        physY = screenHeight - physY - physHeight;
        
        clipRectStack.beginClipping(physX, physY, physWidth, physHeight);
    }
    
    void endClipping()
    {
        clipRectStack.endClipping();
    }
    
    void setBaseTransform(const Transform& baseTransform)
    {
        assert (individualTransforms.size() == 1);
        assert (absoluteTransforms.size() == 1);
        
        individualTransforms.front() = absoluteTransforms.front() = baseTransform;
    }
    
    void pushTransform(const Transform& transform)
    {
        individualTransforms.push_back(transform);
        Transform result = multiply(transform, currentTransform());
        makeCurrentTransform(result);
    }
    
    void popTransform()
    {
        assert (individualTransforms.size() > 1);
        
        individualTransforms.pop_back();
        // TODO: If currentTransform() wouldn't have to be .back(), then I think
        // this could be optimized away and just be pop_back too. Or not?
        Transform result = scale(1);
        for (Transforms::reverse_iterator it = individualTransforms.rbegin(),
                end = individualTransforms.rend(); it != end; ++it)
            result = multiply(result, *it);
        makeCurrentTransform(result);
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
        {
            manager.setRenderState(current->renderState);
            current->perform(&*(current + 1));
        }
        manager.setRenderState(last->renderState);
        last->perform(0);
        #else
        for (DrawOps::const_iterator current = ops.begin(), last = ops.end();
            current != last; ++current)
        {
            manager.setRenderState(current->renderState);
            if (current->verticesOrBlockIndex >= 0)
                current->perform(0);
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
    
    void compileTo(VertexArrays& vas)
    {
        if (!glBlocks.empty())
            throw std::logic_error("Custom code cannot be recorded into a macro");
        
        std::stable_sort(ops.begin(), ops.end());
        for (DrawOps::const_iterator op = ops.begin(), end = ops.end(); op != end; ++op)
            op->compileTo(vas);
    }
    
    // This retains the current stack of transforms and clippings.
    void clearQueue()
    {
        glBlocks.clear();
        ops.clear();
    }
    
    // This clears the queue and starts with new stacks. This must not be called
    // when endClipping/popTransform calls might still be pending.
    void reset()
    {
        absoluteTransforms.resize(1);
        // Important!! Due to all the swapping, the first entry in the list is not necessarily
        // the base matrix. We need to restore it.
        absoluteTransforms.front() = scale(1);
        individualTransforms.resize(1);
        clipRectStack.clear();
        clearQueue();
    }
};

#endif
