#ifndef GOSUIMPL_GRAPHICS_HPP
#define GOSUIMPL_GRAPHICS_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <boost/array.hpp>
#include <vector>

namespace Gosu
{
    enum BorderFlagsEx
    {
        // Flags that affect the width of a border.
        bfDoubleRight = 16,
        bfDoubleBottom = 32
    };

    void applyBorderFlags(Bitmap& dest, const Bitmap& source,
        unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
        unsigned borderFlags);

    template<typename T>
    class BlitQueue
    {
        boost::array<std::vector<T>, 256> layers;

    public:
        void addBlit(const T& blit, ZPos z)
        {
            layers[z].push_back(blit);
        }

        void performBlits()
        {
            for (unsigned z = 0; z < 256; ++z)
            {
                for (unsigned i = 0; i < layers[z].size(); ++i)
                    layers[z][i].perform();
                layers[z].clear();
            }
        }
    };
}

#endif
