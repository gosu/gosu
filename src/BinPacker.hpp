#pragma once

#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <memory>
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

        std::shared_ptr<Rect> alloc(int width, int height);

    private:
        /// Performs the "Rectangle Merge Improvement" (-RM) by repeatedly merging adjacent free
        /// rects into m_free_rects[index] if they can be replaced by a single, larger rectangle.
        void merge_neighbors(int index);
    };
}
