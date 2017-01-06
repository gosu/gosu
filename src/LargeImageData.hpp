#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Platform.hpp>
#include <memory>
#include <vector>

namespace Gosu
{
    class LargeImageData : public ImageData
    {
        unsigned full_width, full_height;
        unsigned parts_x, parts_y;
        unsigned part_width, part_height;
        std::vector<std::shared_ptr<ImageData>> parts;

    public:
        LargeImageData(const Bitmap& source, unsigned part_width, unsigned part_height,
            unsigned image_flags);

        int width() const override;
        int height() const override;

        void draw(double x1, double y1, Color c1, double x2, double y2, Color c2,
            double x3, double y3, Color c3, double x4, double y4, Color c4, ZPos z,
            AlphaMode mode) const override;

        const GLTexInfo* gl_tex_info() const override
        {
            return nullptr;
        }
        
        std::unique_ptr<ImageData> subimage(int x, int y, int w, int h) const override
        {
            return std::unique_ptr<ImageData>();
        }
        
        Bitmap to_bitmap() const override;
        
        void insert(const Bitmap& bitmap, int x, int y) override;
    };
}
