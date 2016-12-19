#include "BlockAllocator.hpp"
#include <stdexcept>
#include <vector>

struct Gosu::BlockAllocator::Impl
{
    unsigned width, height;

    std::vector<Block> blocks;
    unsigned firstX, firstY;
    unsigned maxW, maxH;
    
    void markBlockUsed(const Block& block, unsigned aWidth, unsigned aHeight)
    {
        firstX += aWidth;
        if (firstX + aWidth >= width)
        {
            firstX = 0;
            firstY += aHeight;
        }
        blocks.push_back(block);
    }

    bool isBlockFree(const Block& block) const
    {
        // right and bottom are exclusive (not part of the block).
        unsigned right = block.left + block.width;
        unsigned bottom = block.top + block.height;

        // Block isn't valid.
        if (right > width || bottom > height)
            return false;

        // Test if the block collides with any existing rects.
        for (auto b : blocks)
            if (b.left < right && block.left < b.left + b.width &&
                b.top < bottom && block.top < b.top + b.height)
            {
                return false;
            }

        return true;
    }
};

Gosu::BlockAllocator::BlockAllocator(unsigned width, unsigned height)
: pimpl(new Impl)
{
    pimpl->width = width;
    pimpl->height = height;

    pimpl->firstX = 0;
    pimpl->firstY = 0;

    pimpl->maxW = width;
    pimpl->maxH = height;
}

Gosu::BlockAllocator::~BlockAllocator()
{
}

unsigned Gosu::BlockAllocator::width() const
{
    return pimpl->width;
}

unsigned Gosu::BlockAllocator::height() const
{
    return pimpl->height;
}

bool Gosu::BlockAllocator::alloc(unsigned aWidth, unsigned aHeight, Block& b)
{
    // The rect wouldn't even fit onto the texture!
    if (aWidth > width() || aHeight > height())
        return false;

    // We know there's no space left.
    if (aWidth > pimpl->maxW && aHeight > pimpl->maxH)
        return false;
    
    // Start to look for a place next to the last returned rect. Chances are
    // good we'll find a place there.
    b = Block(pimpl->firstX, pimpl->firstY, aWidth, aHeight);
    if (pimpl->isBlockFree(b))
    {
        pimpl->markBlockUsed(b, aWidth, aHeight);
        return true;
    }

    // Brute force: Look for a free place on this texture.
    unsigned& x = b.left;
    unsigned& y = b.top;
    for (y = 0; y <= height() - aHeight; y += 16)
        for (x = 0; x <= width() - aWidth; x += 8)
        {
            if (!pimpl->isBlockFree(b))
                continue;

            // Found a nice place!

            // Try to make up for the large for()-stepping.
            while (y > 0 && pimpl->isBlockFree(Block(x, y - 1, aWidth, aHeight)))
                --y;
            while (x > 0 && pimpl->isBlockFree(Block(x - 1, y, aWidth, aHeight)))
                --x;
            
            pimpl->markBlockUsed(b, aWidth, aHeight);
            return true;
        }

    // So there was no space for the bitmap. Remember this for later.
    pimpl->maxW = aWidth - 1;
    pimpl->maxH = aHeight - 1;
    return false;
}

void Gosu::BlockAllocator::block(unsigned left, unsigned top, unsigned width, unsigned height)
{
    pimpl->blocks.push_back(Block(left, top, width, height));
}

void Gosu::BlockAllocator::free(unsigned left, unsigned top, unsigned width, unsigned height)
{
    for (auto it = pimpl->blocks.begin(), end = pimpl->blocks.end(); it != end; ++it)
    {
        if (it->left == left && it->top == top && it->width == width && it->height == height)
        {
            pimpl->blocks.erase(it);
            // Be optimistic again, since we might have deleted the largest/only block.
            pimpl->maxW = pimpl->width - 1;
            pimpl->maxH = pimpl->height - 1;
            return;
        }
    }

    throw std::logic_error("Tried to free an invalid block");
}
