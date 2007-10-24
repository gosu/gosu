#ifndef GOSUIMPL_TEXTURE_HPP
#define GOSUIMPL_TEXTURE_HPP

#include <Gosu/Fwd.hpp>
#include <GosuImpl/Graphics/Graphics.hpp>
#include <GosuImpl/Graphics/BlockAllocator.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class Gosu::Texture
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
        tryAlloc(Graphics& graphics, DrawOpQueue& queue, boost::shared_ptr<Texture> ptr, const Bitmap& bmp,
            unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight, unsigned padding);
    void free(unsigned x, unsigned y);
};

#endif
