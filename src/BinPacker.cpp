#include <Gosu/Utility.hpp>
#include "BinPacker.hpp"
#include <algorithm>

#ifndef NDEBUG
#include <cassert>
#endif

Gosu::BinPacker::BinPacker(int width, int height)
    : m_width(width),
      m_height(height),
      m_free_rects { Rect { 0, 0, width, height } }
{
}

std::shared_ptr<const Gosu::Rect> Gosu::BinPacker::alloc(int width, int height)
{
    const Rect* best_rect = best_free_rect(width, height);

    // We didn't find a single free rectangle that can fit the required size? Exit.
    if (best_rect == nullptr) {
        return nullptr;
    }

    // We found a free area, place the result in the top left corner of it.
    // (Also make sure that the pointer's deleter returns the rectangle. Note: Even though
    // shared_ptr may call its deleter with a nullptr in general, it will not do so here.)
    std::shared_ptr<const Rect> result(new Rect { best_rect->x, best_rect->y, width, height },
                                       [this](const Rect* p) { add_free_rect(*p); });

    // We need to split the remaining rectangle into two. We use the axis with the longer side.
    // (Called "Longer Axis Split Rule", "-LAS" in the paper.)
    Rect new_rect_right = *best_rect;
    Rect new_rect_below = *best_rect;
    if (best_rect->width < best_rect->height) {
        // ┏━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━┓
        // ┃     result     ┃ new_rect_right ┃
        // ┣━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━┫
        // ┃          new_rect_below         ┃
        // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
        new_rect_right.x += width;
        new_rect_right.width -= width;
        new_rect_right.height = height;

        new_rect_below.y += height;
        new_rect_below.height -= height;
    }
    else {
        // ┏━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━┓
        // ┃    result      ┃                ┃
        // ┣━━━━━━━━━━━━━━━━┫ new_rect_right ┃
        // ┃ new_rect_below ┃                ┃
        // ┗━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━┛
        new_rect_right.x += width;
        new_rect_right.width -= width;

        new_rect_below.y += height;
        new_rect_below.height -= height;
        new_rect_below.width = width;
    }

    remove_free_rect(static_cast<int>(best_rect - m_free_rects.data()));

    if (!new_rect_below.empty()) {
        add_free_rect(new_rect_below);
    }
    if (!new_rect_right.empty()) {
        add_free_rect(new_rect_right);
    }

    return result;
}

void Gosu::BinPacker::add_free_rect(const Rect& rect)
{
#ifndef NDEBUG
    for (const Rect& other_free_rect : m_free_rects) {
        assert(!rect.overlaps(other_free_rect));
    }
#endif

    m_free_rects.push_back(rect);
    merge_neighbors(static_cast<int>(m_free_rects.size() - 1));
}

const Gosu::Rect* Gosu::BinPacker::best_free_rect(int width, int height) const
{
    // The rect wouldn't even fit onto the texture!
    if (width > m_width || height > m_height) {
        return nullptr;
    }

    const Rect* best_rect = nullptr;
    int best_weight = 0;

    for (const Rect& free_rect : m_free_rects) {
        // This implements the "Best Short Side Fit" (BSSF) metric for choosing a free area.
        int weight = std::min(free_rect.width - width, free_rect.height - height);
        if (weight < 0) {
            continue;
        }
        if (best_rect == nullptr || weight < best_weight) {
            best_rect = &free_rect;
            best_weight = weight;
        }
    }

    return best_rect;
}

void Gosu::BinPacker::remove_free_rect(int index, int* other_index)
{
    std::swap(m_free_rects.at(index), m_free_rects.back());
    if (other_index && *other_index == m_free_rects.size() - 1) {
        *other_index = index;
    }

    m_free_rects.pop_back();
}

void Gosu::BinPacker::merge_neighbors(int index)
{
    // This algorithm tries to merge adjacent free rectangles into larger ones where possible.
    // However, it only finds pairwise combinations of rectangles that can be merged into a single,
    // larger rectangle. You may find that textures/bins sometimes end up with free rectangles that
    // are shaped like this:
    //   ┏━━━━┳━┓
    //   ┣━┳━━┫ ┃
    //   ┃ ┣━━┻━┫
    //   ┗━┻━━━━┛
    // In this case, the texture gets stuck in this fragmented state. We assume that this is not an
    // issue in practice, just like memory

    // Merge any of the other rectangles in the list into the one with the given index if they share
    // any of their four sides.
    for (int j = 0; j < m_free_rects.size(); ++j) {
        Rect& rect = m_free_rects[index];

        const Rect& other_rect = m_free_rects[j];
        bool merged = false;
        if (rect.x == other_rect.x && rect.width == other_rect.width) {
            // Both rectangles are horizontally aligned.

            if (rect.y == other_rect.bottom()) {
                // rect is directly below other_rect, expand it upward.
                rect.y = other_rect.y;
                rect.height += other_rect.height;
                merged = true;
            }
            else if (other_rect.y == rect.bottom()) {
                // rect is directly above other_rect, expand it downward.
                rect.height += other_rect.height;
                merged = true;
            }
        }
        else if (rect.y == other_rect.y && rect.height == other_rect.height) {
            // Both rectangles are vertically aligned.

            if (rect.x == other_rect.right()) {
                // rect is directly to the right of other_rect, expand it to the left.
                rect.x = other_rect.x;
                rect.width += other_rect.width;
                merged = true;
            }
            else if (other_rect.x == rect.right()) {
                // rect is directly to the left of other_rect, expand it to the right.
                rect.width += other_rect.width;
                merged = true;
            }
        }

        // If we merged two rectangles, then we need to start over because the longer sides of the
        // combined rectangle might allow new mergers with other adjacent rectangles, and so on.
        if (merged) {
            remove_free_rect(j, &index);
            // Reset j to -1 so that the next loop iteration will resume at j == 0.
            j = -1;
        }
    }
}
