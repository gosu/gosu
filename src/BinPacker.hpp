#pragma once

#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <mutex>
#include <memory>
#include <vector>

namespace Gosu
{
    /// This implements an allocator for 2D rects in a given "bin", which is typically an OpenGL
    /// texture. It uses the GUILLOTINE-LAS-RM-BSSF algorithm because it has the best worst-case
    /// performance in the PDF paper found here: https://github.com/juj/RectangleBinPack
    /// (The extra complexity of the MAXRECTS algorithm did not seem to be worth it, and the Skyline
    /// algorithm does not seem well-suited for scenarios where images are occasionally deleted.)
    ///
    /// Note: This class cannot use stb_rect_pack.h because that uses an "offline" algorithm,
    /// i.e. it requires all boxes to be allocated at the same time, which is not how Gosu games are
    /// usually structured.
    ///
    /// (This class is non-copyable because alloc returns shared_ptrs that reference this object.
    /// Moving a BinPacker instance would lead to dangling pointers.)
    class BinPacker : private Noncopyable
    {
        int m_width, m_height;
        std::vector<Rect> m_free_rects;
        std::recursive_mutex m_mutex;

    public:
        BinPacker(int width, int height);

        int width() const { return m_width; }
        int height() const { return m_height; }

        /// Finds a free rectangle in the bin and marks it as used, or returns nullptr.
        /// The returned shared_ptr will automatically mark the rectangle as freed through its
        /// deleter. The shared_ptr must not outlive the BinPacker.
        std::shared_ptr<const Rect> alloc(int width, int height);
        /// Marks a previously allocated rectangle as free again. This must be called with one of
        /// the rectangles previously returned by alloc().
        void add_free_rect(const Rect& rect);

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
