#pragma once

#include <Gosu/Fwd.hpp>
#include "BinPacker.hpp"
#include "TexChunk.hpp"
#include <memory>

namespace Gosu
{
    class Texture : public std::enable_shared_from_this<Texture>, Noncopyable
    {
        BinPacker m_bin_packer;
        GLuint m_tex_name;
        bool m_retro;

    public:
        Texture(int width, int height, bool retro);
        ~Texture();

        int width() const { return m_bin_packer.width(); }
        int height() const { return m_bin_packer.height(); }
        GLuint tex_name() const { return m_tex_name; };
        bool retro() const { return m_retro; }

        std::unique_ptr<TexChunk> try_alloc(const Bitmap& bitmap, int padding);
        void block(const Rect& rect);
        void free(const Rect& rect);

        Bitmap to_bitmap(const Rect& rect) const;
    };

}
