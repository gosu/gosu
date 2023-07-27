#pragma once

#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <optional>
#include <vector>

namespace Gosu
{
    /// This implements an allocator for 2D rects in a given "bin", which is typically an OpenGL
    /// texture. It uses the GUILLOTINE-LAS-RM-BSSF algorithm because it has the best worst-case
    /// performance in the PDF paper found here: https://github.com/juj/RectangleBinPack
    /// (The extra complexity of the MAXRECTS algorithm did not seem to be worth it.)
    ///
    /// Note: This class cannot use stb_rect_pack.h because that uses an "offline" algorithm,
    /// i.e. it requires all boxes to be allocated at the same time, which is not how Gosu games are
    /// usually structured.
    class BinPacker
    {
        int m_width, m_height;
        std::vector<Rect> m_free_rects;

    public:
        BinPacker(int width, int height);

        int width() const { return m_width; }
        int height() const { return m_height; }

        /// Finds a free rectangle in the bin and marks it as used, or returns std::nullopt.
        std::optional<Rect> alloc(int width, int height);
        /// Marks a previously allocated rectangle as free again. This must be called with one of
        /// the rectangles previously returned by alloc().
        void free(const Rect& rect);

    private:
        /// Finds the best free rectangle using the "Best Short Side Fit" ("BSSF") metric, if any.
        const Rect* best_free_rect(int width, int height) const;

        /// Removes m_free_rects[index]. If a pointer to another index is given, it will be adjusted
        /// to the new index of the previously pointed-to rectangle (if it has moved).
        void remove_free_rect(int index, int* other_index = nullptr);

        /// Performs the "Rectangle Merge Improvement" (-RM) by repeatedly merging adjacent free
        /// rects into m_free_rects[index] if they can be replaced by a single, larger rectangle.
        void merge_neighbors(int index);
    };
}
