#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Bitmap.hpp>
#include "GraphicsImpl.hpp"
#include "TexChunk.hpp"
#include "BlockAllocator.hpp"
#include <vector>

namespace Gosu
{
    class Texture
    {
        BlockAllocator allocator_;
        GLuint texName_;
        bool retro_;

    public:
        Texture(unsigned size, bool retro);
        ~Texture();
        unsigned size() const;
        GLuint texName() const;
        bool retro() const;
        std::unique_ptr<TexChunk>
            tryAlloc(std::shared_ptr<Texture> ptr, const Bitmap& bmp, unsigned padding);
        void block(unsigned x, unsigned y, unsigned width, unsigned height);
        void free(unsigned x, unsigned y, unsigned width, unsigned height);
        Gosu::Bitmap toBitmap(unsigned x, unsigned y, unsigned width, unsigned height) const;
    };
}
