#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Drawable.hpp>
#include <memory>

namespace Gosu
{
    class DrawOpQueue;

    class Macro : public Drawable
    {
        // This is a shared_ptr so that enqueued DrawOps can keep the macro data alive even after
        // the Macro object itself has been destroyed.
        struct Impl;
        std::shared_ptr<const Impl> pimpl;

    public:
        Macro(DrawOpQueue& queue, int width, int height);

        int width() const override;
        int height() const override;

        void draw(double x1, double y1, Color c1, double x2, double y2, Color c2, //
                  double x3, double y3, Color c3, double x4, double y4, Color c4, //
                  ZPos z, BlendMode mode) const override;

        const GLTexInfo* gl_tex_info() const override;

        Bitmap to_bitmap() const override;

        std::unique_ptr<Drawable> subimage(const Rect&) const override;

        void insert(const Bitmap&, int x, int y) override;
    };
}
