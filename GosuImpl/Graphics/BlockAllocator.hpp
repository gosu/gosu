#ifndef GOSUIMPL_BLOCKALLOCATOR_HPP
#define GOSUIMPL_BLOCKALLOCATOR_HPP

#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>

namespace Gosu
{
    class BlockAllocator
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        struct Block
        {
            unsigned left, top, width, height;
            Block() {}
            Block(unsigned aLeft, unsigned aTop, unsigned aWidth, unsigned aHeight)
            : left(aLeft), top(aTop), width(aWidth), height(aHeight) {}
        };
    
        BlockAllocator(unsigned width, unsigned height);
        ~BlockAllocator();

        unsigned width() const;
        unsigned height() const;

        boost::optional<Block> alloc(unsigned width, unsigned height);
        void free(unsigned left, unsigned top);
    };
}

#endif
