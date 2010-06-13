#ifndef GOSUIMPL_GRAPHICS_TEXTURE_HPP
#define GOSUIMPL_GRAPHICS_TEXTURE_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Bitmap.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <GosuImpl/Graphics/BlockAllocator.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Gosu
{
    class Texture
    {
        BlockAllocator allocator;
        GLuint name;
        unsigned num;

    public:
        static unsigned maxTextureSize();

        Texture(unsigned size);
        ~Texture();
        unsigned size() const;
        GLuint texName() const;
        std::auto_ptr<TexChunk> 
            tryAlloc(Graphics& graphics, Transforms& transforms, DrawOpQueueStack& queues, boost::shared_ptr<Texture> ptr,
                const Bitmap& bmp, unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
                unsigned padding);
        void free(unsigned x, unsigned y);
        Gosu::Bitmap toBitmap(unsigned x, unsigned y, unsigned width, unsigned height) const;
    };
}

#endif
