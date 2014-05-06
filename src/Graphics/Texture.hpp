#ifndef GOSUIMPL_GRAPHICS_TEXTURE_HPP
#define GOSUIMPL_GRAPHICS_TEXTURE_HPP

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
        BlockAllocator allocator;
        GLuint name;

    public:
        Texture(unsigned size);
        ~Texture();
        unsigned size() const;
        GLuint texName() const;
        GOSU_UNIQUE_PTR<TexChunk>
            tryAlloc(Graphics& graphics, DrawOpQueueStack& queues,
                std::tr1::shared_ptr<Texture> ptr, const Bitmap& bmp, unsigned padding);
        void block(unsigned x, unsigned y, unsigned width, unsigned height);
        void free(unsigned x, unsigned y, unsigned width, unsigned height);
        Gosu::Bitmap toBitmap(unsigned x, unsigned y, unsigned width, unsigned height) const;
    };
}

#endif
