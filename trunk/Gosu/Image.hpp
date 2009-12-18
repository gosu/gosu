//! \file Image.hpp
//! Interface of the Image class and helper functions.

#ifndef GOSU_IMAGE_HPP
#define GOSU_IMAGE_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/RotFlip.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>

namespace Gosu
{
    //! Provides functionality for drawing rectangular images.
    class Image
    {
        boost::scoped_ptr<ImageData> data;

    public:
        //! Loads an image from a given filename that can be drawn onto
        //! graphics.
		//! This constructor can handle PNG and BMP images. A color key of #ff00ff is
		//! automatically applied to BMP type images. For more flexibility, use the
		//! corresponding constructor that uses a Bitmap object.
        Image(Graphics& graphics, const std::wstring& filename,
              bool tileable = false);
        //! Loads a portion of the the image at the given filename that can be
        //! drawn onto graphics.
		//! This constructor can handle PNG and BMP images. A color key of #ff00ff is
		//! automatically applied to BMP type images. For more flexibility, use the
		//! corresponding constructor that uses a Bitmap object.
        Image(Graphics& graphics, const std::wstring& filename, unsigned srcX,
              unsigned srcY, unsigned srcWidth, unsigned srcHeight,
              bool tileable = false);
        
        //! Converts the given bitmap into an image that can be drawn onto
        //! graphics.
        Image(Graphics& graphics, const Bitmap& source,
            bool tileable = false);
        //! Converts a portion of the given bitmap into an image that can be
        //! drawn onto graphics.
        Image(Graphics& graphics, const Bitmap& source, unsigned srcX,
            unsigned srcY, unsigned srcWidth, unsigned srcHeight,
            bool tileable = false);
        
        //! Creates an Image from a user-supplied instance of the ImageData interface.
        explicit Image(std::auto_ptr<ImageData> data);

        ~Image();
        
        unsigned width() const;
        unsigned height() const;

        //! Draws the image so its upper left corner is at (x; y).
        void draw(double x, double y, ZPos z,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE,
            AlphaMode mode = amDefault) const;
        //! Like draw(), but allows to give modulation colors for all four
        //! corners.
        void drawMod(double x, double y, ZPos z,
            double factorX, double factorY,
            Color c1, Color c2, Color c3, Color c4,
            AlphaMode mode = amDefault) const;

        //! Draws the image rotated by the given angle so that its rotation
        //! center is at (x; y). Note that this is different from how all the
        //! other drawing functions work!
        //! \param angle See Math.hpp for an explanation of how Gosu interprets
        //! angles.
        //! \param centerX Relative horizontal position of the rotation center
        //! on the image. 0 is the left border, 1 is the right border, 0.5 is
        //! the center (and default).
        //! \param centerY See centerX.
        void drawRot(double x, double y, ZPos z,
            double angle, double centerX = 0.5, double centerY = 0.5,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE,
            AlphaMode mode = amDefault) const;

        #ifndef SWIG
        void drawRotFlip(double x, double y, ZPos z,
            RotFlip rf,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE,
            AlphaMode mode = amDefault) const;
        void drawRotFlipMod(double x, double y, ZPos z,
            RotFlip rf,
            double factorX, double factorY,
            Color c1, Color c2, Color c3, Color c4,
            AlphaMode mode = amDefault) const;

        //! Provides access to the underlying image data object.
        const ImageData& getData() const;
        #endif
    };

	//! Convenience function that splits a BMP or PNG file into an array
	//! of small rectangles and creates images from them.
    //! \param tileWidth If positive, specifies the width of one tile in
    //! pixels. If negative, the bitmap is divided into -tileWidth rows.
    //! \param tileHeight See tileWidth.
    //! \param appendTo STL container to which the images will be appended.
    //! Must provide a push_back member function; std::vector<boost::shared_ptr<Image>>
    //! or boost::ptr_vector<Image> are good choices.
    template<typename Container>
    void imagesFromTiledBitmap(Graphics& graphics, const std::wstring& filename,
        int tileWidth, int tileHeight, bool tileable, Container& appendTo)
    {
		imagesFromTiledBitmap(graphics, quickLoadBitmap(filename), tileWidth, tileHeight, tileable, appendTo);
    }

    //! Convenience function that splits a bitmap into an area of array 
    //! rectangles and creates images from them.
    //! \param tileWidth If positive, specifies the width of one tile in
    //! pixels. If negative, the bitmap is divided into -tileWidth rows.
    //! \param tileHeight See tileWidth.
    //! \param appendTo STL container to which the images will be appended.
    //! Must provide a push_back member function; std::vector<boost::shared_ptr<Image>>
    //! or boost::ptr_vector<Image> are good choices.
    template<typename Container>
    void imagesFromTiledBitmap(Graphics& graphics, const Bitmap& bmp,
        int tileWidth, int tileHeight, bool tileable, Container& appendTo)
    {
        int tilesX, tilesY;

        if (tileWidth > 0)
            tilesX = bmp.width() / tileWidth;
        else
        {
            tilesX = -tileWidth;
            tileWidth = bmp.width() / tilesX;
        }
        
        if (tileHeight > 0)
            tilesY = bmp.height() / tileHeight;
        else
        {
            tilesY = -tileHeight;
            tileHeight = bmp.height() / tilesY;
        }
        
        for (int y = 0; y < tilesY; ++y)
            for (int x = 0; x < tilesX; ++x)
                appendTo.push_back(typename Container::value_type(new Image(graphics, bmp,
                    x * tileWidth, y * tileHeight, tileWidth, tileHeight,
                    tileable)));
    }
}

#endif
