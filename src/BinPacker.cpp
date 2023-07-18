#include <Gosu/Utility.hpp>
#include "BinPacker.hpp"
#include <stdexcept>
#include <vector>

struct Gosu::BinPacker::Impl : private Gosu::Noncopyable
{
    int width, height;

    std::vector<Rect> blocks;
    int first_x, first_y;
    int max_w, max_h;

    Impl(int width, int height)
        : width(width),
          height(height),
          first_x(0),
          first_y(0),
          max_w(width),
          max_h(height)
    {
    }

    void mark_block_used(const Rect& block)
    {
        first_x += block.width;
        if (first_x + block.width >= width) {
            first_x = 0;
            first_y += block.height;
        }
        blocks.push_back(block);
    }

    bool is_block_free(const Rect& block) const
    {
        // right and bottom are exclusive (not part of the block).
        int right = block.x + block.width;
        int bottom = block.y + block.height;

        // Block isn't valid.
        if (right > width || bottom > height) {
            return false;
        }

        // Test if the block collides with any existing rects.
        for (const auto& b : blocks) {
            if (b.x < right && block.x < b.x + b.width && b.y < bottom
                && block.y < b.y + b.height) {
                return false;
            }
        }

        return true;
    }
};

Gosu::BinPacker::BinPacker(int width, int height)
    : pimpl(new Impl(width, height))
{
    pimpl->width = width;
    pimpl->height = height;
}

Gosu::BinPacker::~BinPacker()
{
}

int Gosu::BinPacker::width() const
{
    return pimpl->width;
}

int Gosu::BinPacker::height() const
{
    return pimpl->height;
}

std::optional<Gosu::Rect> Gosu::BinPacker::find_rect(int width, int height)
{
    // The rect wouldn't even fit onto the texture!
    if (width > pimpl->width || height > pimpl->height) {
        return std::nullopt;
    }

    // We know there's no space left.
    if (width > pimpl->max_w && height > pimpl->max_h) {
        return std::nullopt;
    }

    // Start to look for a place next to the last returned rect. Chances are
    // good we'll find a place there.
    Rect b { pimpl->first_x, pimpl->first_y, width, height };
    if (pimpl->is_block_free(b)) {
        return b;
    }

    // Brute force: Look for a free place on this texture.
    for (b.y = 0; b.y <= pimpl->height - height; b.y += 16) {
        for (b.x = 0; b.x <= pimpl->width - width; b.x += 8) {
            if (!pimpl->is_block_free(b)) {
                continue;
            }

            // Found a nice place!

            // Try to make up for the large for ()-stepping.
            while (b.y > 0 && pimpl->is_block_free(Rect { b.x, b.y - 1, b.width, b.height })) {
                --b.y;
            }
            while (b.x > 0 && pimpl->is_block_free(Rect { b.x - 1, b.y, b.width, b.height })) {
                --b.x;
            }

            return b;
        }
    }

    // So there was no space for the bitmap. Remember this for later.
    pimpl->max_w = width - 1;
    pimpl->max_h = height - 1;
    return std::nullopt;
}

void Gosu::BinPacker::block(const Rect& rect)
{
    pimpl->mark_block_used(rect);
}

void Gosu::BinPacker::free(const Rect& rect)
{
    const auto iterator = std::find(pimpl->blocks.begin(), pimpl->blocks.end(), rect);
    if (iterator == pimpl->blocks.end()) {
        throw std::logic_error("Tried to free an invalid block");
    }
    pimpl->blocks.erase(iterator);
    // Be optimistic again, since we might have deleted the largest/only block.
    pimpl->max_w = pimpl->width - 1;
    pimpl->max_h = pimpl->height - 1;
}
