#pragma once

#include "BlockAllocator.hpp"
#include "GraphicsImpl.hpp"
#include "TexChunk.hpp"
#include <Gosu/Fwd.hpp>
#include <Gosu/Bitmap.hpp>
#include <memory>
#include <vector>

class Gosu::Texture : public std::enable_shared_from_this<Texture>
{
    // BlockAllocator can't be copied or moved, so neither can Texture.
    BlockAllocator allocator_;
    GLuint tex_name_;
    bool retro_;
    
public:
    Texture(unsigned size, bool retro);
    ~Texture();
    unsigned size() const;
    GLuint tex_name() const;
    bool retro() const;
    std::unique_ptr<TexChunk> try_alloc(const Bitmap& bmp, unsigned padding);
    void block(unsigned x, unsigned y, unsigned width, unsigned height);
    void free(unsigned x, unsigned y, unsigned width, unsigned height);
    Bitmap to_bitmap(unsigned x, unsigned y, unsigned width, unsigned height) const;
};
