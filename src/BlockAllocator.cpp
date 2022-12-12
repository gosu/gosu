#include <Gosu/Utility.hpp>
#include "BlockAllocator.hpp"
#include <stdexcept>
#include <vector>
using namespace std;

struct Gosu::BlockAllocator::Impl : private Gosu::Noncopyable
{
    unsigned width, height;

    vector<Block> blocks;
    unsigned first_x, first_y;
    unsigned max_w, max_h;
    
    void mark_block_used(const Block& block, unsigned a_width, unsigned a_height)
    {
        first_x += a_width;
        if (first_x + a_width >= width) {
            first_x = 0;
            first_y += a_height;
        }
        blocks.push_back(block);
    }

    bool is_block_free(const Block& block) const
    {
        // right and bottom are exclusive (not part of the block).
        unsigned right = block.left + block.width;
        unsigned bottom = block.top + block.height;

        // Block isn't valid.
        if (right > width || bottom > height) return false;

        // Test if the block collides with any existing rects.
        for (auto b : blocks) {
            if (b.left < right && block.left < b.left + b.width &&
                    b.top < bottom && block.top < b.top + b.height) {
                return false;
            }
        }

        return true;
    }
};

Gosu::BlockAllocator::BlockAllocator(unsigned width, unsigned height)
: pimpl(new Impl)
{
    pimpl->width = width;
    pimpl->height = height;

    pimpl->first_x = 0;
    pimpl->first_y = 0;

    pimpl->max_w = width;
    pimpl->max_h = height;
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

bool Gosu::BlockAllocator::alloc(unsigned a_width, unsigned a_height, Block& b)
{
    // The rect wouldn't even fit onto the texture!
    if (a_width > width() || a_height > height()) return false;

    // We know there's no space left.
    if (a_width > pimpl->max_w && a_height > pimpl->max_h) return false;
    
    // Start to look for a place next to the last returned rect. Chances are
    // good we'll find a place there.
    b = Block(pimpl->first_x, pimpl->first_y, a_width, a_height);
    if (pimpl->is_block_free(b)) {
        pimpl->mark_block_used(b, a_width, a_height);
        return true;
    }

    // Brute force: Look for a free place on this texture.
    unsigned& x = b.left;
    unsigned& y = b.top;
    for (y = 0; y <= height() - a_height; y += 16) {
        for (x = 0; x <= width() - a_width; x += 8) {
            if (!pimpl->is_block_free(b)) continue;

            // Found a nice place!

            // Try to make up for the large for ()-stepping.
            while (y > 0 && pimpl->is_block_free(Block(x, y - 1, a_width, a_height))) --y;
            while (x > 0 && pimpl->is_block_free(Block(x - 1, y, a_width, a_height))) --x;
            
            pimpl->mark_block_used(b, a_width, a_height);
            return true;
        }
    }

    // So there was no space for the bitmap. Remember this for later.
    pimpl->max_w = a_width - 1;
    pimpl->max_h = a_height - 1;
    return false;
}

void Gosu::BlockAllocator::block(unsigned left, unsigned top, unsigned width, unsigned height)
{
    pimpl->blocks.emplace_back(left, top, width, height);
}

void Gosu::BlockAllocator::free(unsigned left, unsigned top, unsigned width, unsigned height)
{
    for (auto it = pimpl->blocks.begin(), end = pimpl->blocks.end(); it != end; ++it) {
        if (it->left == left && it->top == top && it->width == width && it->height == height) {
            pimpl->blocks.erase(it);
            // Be optimistic again, since we might have deleted the largest/only block.
            pimpl->max_w = pimpl->width - 1;
            pimpl->max_h = pimpl->height - 1;
            return;
        }
    }

    throw logic_error("Tried to free an invalid block");
}
