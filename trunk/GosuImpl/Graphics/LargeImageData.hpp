#ifndef GOSUIMPL_LARGEIMAGEDATA_HPP
#define GOSUIMPL_LARGEIMAGEDATA_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <boost/none.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Gosu
{
    class LargeImageData : public ImageData
    {
        unsigned fullWidth, fullHeight, partsX, partsY, partWidth, partHeight;
        std::vector<boost::shared_ptr<ImageData> > parts;

    public:
        LargeImageData(Graphics& graphics, const Bitmap& source,
            unsigned partWidth, unsigned partHeight, unsigned borderFlags);

        unsigned width() const;
        unsigned height() const;

        void draw(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode) const;
            
        boost::optional<GLTexInfo> glTexInfo() const
        {
            return boost::none;
        }
        
        Bitmap toBitmap() const;
    };
}

#endif
