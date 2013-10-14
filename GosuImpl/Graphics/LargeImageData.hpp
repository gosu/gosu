#ifndef GOSUIMPL_LARGEIMAGEDATA_HPP
#define GOSUIMPL_LARGEIMAGEDATA_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Platform.hpp>
#include <stdexcept>
#include <vector>

namespace Gosu
{
    class LargeImageData : public ImageData
    {
        unsigned fullWidth, fullHeight, partsX, partsY, partWidth, partHeight;
        std::vector<std::tr1::shared_ptr<ImageData> > parts;

    public:
        LargeImageData(Graphics& graphics, const Bitmap& source,
            unsigned partWidth, unsigned partHeight, unsigned borderFlags);

        int width() const;
        int height() const;

        void draw(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode) const;
            
        const GLTexInfo* glTexInfo() const
        {
            return 0;
        }
        
        GOSU_UNIQUE_PTR<ImageData> subimage(int x, int y, int w, int h) const
        {
            return GOSU_UNIQUE_PTR<ImageData>();
        }
        
        Bitmap toBitmap() const;
        void insert(const Bitmap& bitmap, int x, int y);
    };
}

#endif
