#pragma once

#include <Gosu/ImageData.hpp>
#include <memory>

namespace Gosu
{
    class EmptyImageData : public ImageData
    {
    public:
        int width() const override //
        {
            return 0;
        }

        int height() const override //
        {
            return 0;
        }

        void draw(double, double, Color, double, double, Color, //
                  double, double, Color, double, double, Color, //
                  ZPos, BlendMode) const override
        {
        }

        GLTexInfo* gl_tex_info() const override //
        {
            return nullptr;
        }

        Bitmap to_bitmap() const override //
        {
            return Bitmap();
        }

        std::unique_ptr<ImageData> subimage(int, int, int, int) const override //
        {
            return nullptr;
        }

        void insert(const Bitmap&, int x, int y) override //
        {
        }

        static const std::shared_ptr<EmptyImageData>& instance_ptr()
        {
            static std::shared_ptr<EmptyImageData> instance = std::make_shared<EmptyImageData>();
            return instance;
        }
    };
}
