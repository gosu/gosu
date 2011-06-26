#ifndef GOSUIMPL_BLOCKALLOCATOR_HPP
#define GOSUIMPL_BLOCKALLOCATOR_HPP

#include <memory>

namespace Gosu
{
    class BlockAllocator
    {
        struct Impl;
        const std::auto_ptr<Impl> pimpl;

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

        bool alloc(unsigned width, unsigned height, Block& block);
        void free(unsigned left, unsigned top);
    };
}

#endif
