#pragma once

#include "BlockAllocator.hpp"
#include "GraphicsImpl.hpp"
#include "TexChunk.hpp"
#include <Gosu/Fwd.hpp>
#include <Gosu/Bitmap.hpp>
#include <vector>

namespace Gosu
{
    class Texture
    {
        BlockAllocator allocator_;
        GLuint tex_name_;
        bool retro_;

    public:
        Texture(unsigned size, bool retro);
        ~Texture();
        unsigned size() const;
        GLuint tex_name() const;
        bool retro() const;
        std::unique_ptr<TexChunk>
            try_alloc(std::shared_ptr<Texture> ptr, const Bitmap& bmp, unsigned padding);
        void block(unsigned x, unsigned y, unsigned width, unsigned height);
        void free(unsigned x, unsigned y, unsigned width, unsigned height);
        Gosu::Bitmap to_bitmap(unsigned x, unsigned y, unsigned width, unsigned height) const;
    };
}
