#pragma once

#include <Gosu/Platform.hpp>
#include <memory>

namespace Gosu
{
    class BlockAllocator
    {
        struct Impl;
        const std::unique_ptr<Impl> pimpl;

    public:
        struct Block
        {
            unsigned left, top, width, height;
            Block() {}
            Block(unsigned left, unsigned top, unsigned width, unsigned height)
            : left(left), top(top), width(width), height(height) {}
        };

        BlockAllocator(unsigned width, unsigned height);
        ~BlockAllocator();

        unsigned width() const;
        unsigned height() const;

        bool alloc(unsigned width, unsigned height, Block& block);
        void block(unsigned left, unsigned top, unsigned width, unsigned height);
        void free(unsigned left, unsigned top, unsigned width, unsigned height);
    };
}
