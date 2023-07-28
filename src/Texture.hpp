#pragma once

#include <Gosu/Fwd.hpp>
#include "BinPacker.hpp"
#include "TexChunk.hpp"
#include <cstdint>
#include <memory>

namespace Gosu
{
    class Texture : public std::enable_shared_from_this<Texture>, private Noncopyable
    {
        BinPacker m_bin_packer;
        std::uint32_t m_tex_name;
        bool m_retro;

    public:
        Texture(int width, int height, bool retro);
        ~Texture();

        int width() const { return m_bin_packer.width(); }
        int height() const { return m_bin_packer.height(); }
        std::uint32_t tex_name() const { return m_tex_name; }
        bool retro() const { return m_retro; }

        [[nodiscard]] std::unique_ptr<TexChunk> try_alloc(const Bitmap& bitmap, int padding);

        Bitmap to_bitmap(const Rect& rect) const;
    };
}
