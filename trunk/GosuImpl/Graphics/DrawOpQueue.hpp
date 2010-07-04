#ifndef GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP
#define GOSUIMPL_GRAPHICS_DRAWOPQUEUE_HPP

#include <GosuImpl/Graphics/Common.hpp>
#include <algorithm>
#include <set>
#include <boost/foreach.hpp>

class Gosu::DrawOpQueue
{
    int clipX, clipY;
    unsigned clipWidth, clipHeight;
    std::multiset<DrawOp> set;

public:
    DrawOpQueue()
    : clipWidth(NO_CLIPPING)
    {
    }
    
    void swap(DrawOpQueue& other)
    {
        std::swap(clipX, other.clipX);
        std::swap(clipY, other.clipY);
        std::swap(clipWidth, other.clipWidth);
        std::swap(clipHeight, other.clipHeight);
        set.swap(other.set);
    }
    
    void addDrawOp(DrawOp op, ZPos z)
    {
        #ifdef GOSU_IS_IPHONE
        // No triangles, no lines supported
        assert(op.usedVertices == 4);
        #endif

        if (clipWidth != NO_CLIPPING)
        {
            op.clipX = clipX;
            op.clipY = clipY;
            op.clipWidth = clipWidth;
            op.clipHeight = clipHeight;
        }
        
        op.z = z;
        set.insert(op);
    }
    
    void beginClipping(int x, int y, unsigned width, unsigned height)
    {
        clipX = x;
        clipY = y;
        clipWidth = width;
        clipHeight = height;
    }
    
    void endClipping()
    {
        clipWidth = NO_CLIPPING;
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
