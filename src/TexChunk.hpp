#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Utility.hpp>
#include <cstdint>
#include <memory>

namespace Gosu
{
    class Texture;

    /// The most common Drawable implementation which uses a portion of, or a full, OpenGL texture
    /// to store image data.
    class TexChunk : public Drawable
    {
        const std::shared_ptr<Texture> m_texture;
        const Rect m_rect;
        const GLTexInfo m_info;
        const std::shared_ptr<const Rect> m_rect_handle;

    public:
        /// @param texture The texture on which the image data resides.
        /// @param rect The portion of the texture that will be represented by this TexChunk.
        ///             This excludes any padding pixels.
        /// @param rect_handle A shared_ptr that references the full rectangle that was allocated
        ///                    for this TexChunk, including padding. When this TexChunk and all
        ///                    of its subimages have been deleted, this rectangle will be reclaimed
        ///                    for use by other image data.
        TexChunk(const std::shared_ptr<Texture>& texture, const Rect& rect,
                 const std::shared_ptr<const Rect>& rect_handle);

        int width() const override { return m_rect.width; }
        int height() const override { return m_rect.height; }

        std::uint32_t tex_name() const { return m_info.tex_name; }

        void draw(double x1, double y1, Color c1, //
                  double x2, double y2, Color c2, //
                  double x3, double y3, Color c3, //
                  double x4, double y4, Color c4, //
                  ZPos z, BlendMode mode) const override;

        const GLTexInfo* gl_tex_info() const override { return &m_info; }

        std::unique_ptr<Drawable> subimage(const Rect& rect) const override;

        Bitmap to_bitmap() const override;

        void insert(const Bitmap& bitmap, int x, int y) override;
    };
}
