#include <Gosu/Utility.hpp>
#include "BinPacker.hpp"
#include <algorithm>

Gosu::BinPacker::BinPacker(int width, int height)
    : m_width(width),
      m_height(height),
      m_free_rects { Rect { 0, 0, width, height } }
{
}

std::shared_ptr<Gosu::Rect> Gosu::BinPacker::alloc(int width, int height)
{
    // The rect wouldn't even fit onto the texture!
    if (width > m_width || height > m_height) {
        return nullptr;
    }

    Rect* best_rect = nullptr;
    int best_weight = 0;

    for (Rect& free_rect : m_free_rects) {
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

    // We didn't find a single free rectangle that can fit the required size? Exit.
    if (best_rect == nullptr) {
        return nullptr;
    }

    auto free_rect = [this](const Rect* rect) {
        if (rect) {
            m_free_rects.push_back(*rect);
            merge_neighbors(m_free_rects.size() - 1);
        }
    };
    // We found a free area, place the result in the top left corner of it.
    std::shared_ptr<Rect> result(new Rect { best_rect->x, best_rect->y, width, height }, free_rect);

    // We need to split the remaining rectangle into two. We use the axis with the longer side.
    // (Called "Longer Axis Split Rule", "-LAS" in the paper.)
    if (best_rect->width < best_rect->height) {
        // result | best_rect'
        // -------------------
        //   new_rect_below

        const Rect new_rect_below {
            .x = best_rect->x,
            .y = best_rect->y + height,
            .width = best_rect->width,
            .height = best_rect->height - height,
        };

        best_rect->x += width;
        best_rect->width -= width;
        best_rect->height = height;
        if (best_rect->width == 0) {
            m_free_rects.erase(m_free_rects.begin() + (best_rect - m_free_rects.data()));
        }
        else {
            merge_neighbors(best_rect - m_free_rects.data());
        }

        if (new_rect_below.height != 0) {
            free_rect(&new_rect_below);
        }
    } else {
        // result     | new_rect_right
        // -----------|
        // best_rect' |

        const Rect new_rect_right {
            .x = best_rect->x + width,
            .y = best_rect->y,
            .width = best_rect->width - width,
            .height = best_rect->height,
        };

        best_rect->y += height;
        best_rect->height -= height;
        best_rect->width = width;
        if (best_rect->height == 0) {
            m_free_rects.erase(m_free_rects.begin() + (best_rect - m_free_rects.data()));
        }
        else {
            merge_neighbors(best_rect - m_free_rects.data());
        }

        if (new_rect_right.width != 0) {
            free_rect(&new_rect_right);
        }
    }

    return result;
}

void Gosu::BinPacker::merge_neighbors(int index)
{
    // Merge any of the other rectangles in the list into the one with the given index if they share
    // any of their four sides.
    for (int j = 0; j < m_free_rects.size(); ++j) {
        Rect& rect = m_free_rects[index];

        const Rect& other_rect = m_free_rects[j];
        bool merged = false;
        if (rect.x == other_rect.x && rect.width == other_rect.width) {
            // Both rectangles are horizontally aligned.

            if (rect.y == other_rect.y + other_rect.height) {
                // rect is directly below other_rect, expand it upward.
                rect.y = other_rect.y;
                rect.height += other_rect.height;
                merged = true;
            }
            else if (other_rect.y == rect.y + rect.height) {
                // rect is directly above other_rect, expand it downward.
                rect.height += other_rect.height;
                merged = true;
            }
        }
        else if (rect.y == other_rect.y && rect.height == other_rect.height) {
            // Both rectangles are vertically aligned.

            if (rect.x == other_rect.x + other_rect.width) {
                // rect is directly to the right of other_rect, expand it to the left.
                rect.x = other_rect.x;
                rect.width += other_rect.width;
                merged = true;
            }
            else if (other_rect.x == rect.x + rect.width) {
                // rect is directly to the left of other_rect, expand it to the right.
                rect.width += other_rect.width;
                merged = true;
            }
        }

        // If we merged two rectangles, then we need to start over because the longer sides of the
        // combined rectangle might allow new mergers with other adjacent rectangles, and so on.
        if (merged) {
            m_free_rects.erase(m_free_rects.begin() + j);
            if (j < index) {
                --index;
            }
            j = -1;
        }
    }
}
