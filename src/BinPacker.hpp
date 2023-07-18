#pragma once

#include <Gosu/Platform.hpp>
#include <memory>
#include <optional>

namespace Gosu
{
    class BinPacker
    {
        struct Impl;
        const std::unique_ptr<Impl> pimpl;

    public:
        BinPacker(int width, int height);
        ~BinPacker();

        int width() const;
        int height() const;

        std::optional<Rect> find_rect(int width, int height);
        void block(const Rect& rect);
        void free(const Rect& rect);
    };
}
