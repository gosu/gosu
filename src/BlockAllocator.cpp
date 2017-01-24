#include "BlockAllocator.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>

struct Gosu::BlockAllocator::Impl
{
    unsigned width, height;
    unsigned avail_w, avail_h;

    std::vector<Block> blocks;
    std::vector<unsigned> used; // Height table.

    // Rebuild the height table where b was allocated.
    void recalculate_used(Block b)
    {
        std::fill_n(used.begin() + b.left, b.width, 0);
        for (auto i = blocks.begin(); i != blocks.end(); ++i) {
            if (i->left < b.left + b.width && b.left < i->left + i->width) {
                unsigned inter_left = std::max(i->left, b.left);
                unsigned inter_right = std::min(i->left + i->width,
                                                b.left + b.width);
                unsigned ibot = i->top + i->height;
                for (int i = inter_left; i < inter_right; i++){
                    used[i] = std::max(used[i], ibot);
                }
            }
        }
    }
};

Gosu::BlockAllocator::BlockAllocator(unsigned width, unsigned height)
: pimpl(new Impl)
{
    pimpl->width = width;
    pimpl->height = height;

    pimpl->avail_w = width;
    pimpl->avail_h = height;

    auto begin = pimpl->used.begin();
    pimpl->used.insert(begin, width, 0);
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
    unsigned x, i;
    unsigned bestH = height();
    unsigned tentativeH;

    if (a_width > pimpl->avail_w && a_height > pimpl->avail_h) {
        return false;
    }

    // Based on Id Software's block allocator from Quake 2, released under
    // the GPL.
    // See: http://fabiensanglard.net/quake2/quake2_opengl_renderer.php
    // Search for: LM_AllocBlock
    for (x = 0; x <= pimpl->width - a_width; x++) {
        tentativeH = 0;
        for (i = 0; i < a_width; i++) {
            if (pimpl->used[x+i] >= bestH) {
                break;
            }
            if (pimpl->used[x+i] > tentativeH) {
                tentativeH = pimpl->used[x+i];
            }
        }
        if (i == a_width) {
            b.left = x;
            b.top = bestH = tentativeH;
            b.width = a_width;
            b.height = a_height;
        }
    }

    // There wasn't enough space for the bitmap.
    if (bestH + a_height > pimpl->height) {
        // Remember this for later.
        if (a_width > pimpl->avail_w && a_height > pimpl->avail_h) {
            pimpl->avail_w = a_width - 1;
            pimpl->avail_h = a_height - 1;
        }
        return false;
    }

    // We've found a valid spot.
    for (i = 0; i < a_width; i++) {
        pimpl->used[b.left+i] = bestH + a_height;
    }
    pimpl->blocks.push_back(b);

    return true;
}

void Gosu::BlockAllocator::block(unsigned left, unsigned top, unsigned width, unsigned height)
{
    pimpl->blocks.push_back(Block(left, top, width, height));
}

void Gosu::BlockAllocator::free(unsigned left, unsigned top, unsigned width, unsigned height)
{
    for (auto it = pimpl->blocks.begin(), end = pimpl->blocks.end(); it != end; ++it) {
        if (it->left == left && it->top == top && it->width == width && it->height == height) {
            Block b = *it;
            pimpl->blocks.erase(it);
            // Be optimistic again, since we might have deleted the largest/only block.
            pimpl->avail_w = pimpl->width;
            pimpl->avail_h = pimpl->height;
            // Look for freed-up space.
            pimpl->recalculate_used(b);
            return;
        }
    }

    throw std::logic_error("Tried to free an invalid block");
}
