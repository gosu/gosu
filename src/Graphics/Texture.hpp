#ifndef GOSU_SRC_GRAPHICS_TEXTURE_HPP
#define GOSU_SRC_GRAPHICS_TEXTURE_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/TR1.hpp>
#include "Common.hpp"
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
        GOSU_UNIQUE_PTR<TexChunk>
            tryAlloc(std::tr1::shared_ptr<Texture> ptr, const Bitmap& bmp, unsigned padding);
        void block(unsigned x, unsigned y, unsigned width, unsigned height);
        void free(unsigned x, unsigned y, unsigned width, unsigned height);
        Gosu::Bitmap toBitmap(unsigned x, unsigned y, unsigned width, unsigned height) const;
    };
}

#endif
