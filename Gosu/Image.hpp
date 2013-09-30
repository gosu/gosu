//! \file Image.hpp
//! Interface of the Image class and helper functions.

#ifndef GOSU_IMAGE_HPP
#define GOSU_IMAGE_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/TR1.hpp>
#include <memory>
#include <vector>

namespace Gosu
{
    //! Provides functionality for drawing rectangular images.
    class Image
    {
        std::tr1::shared_ptr<ImageData> data;

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
        explicit Image(GOSU_UNIQUE_PTR<ImageData> data);

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

        //! Provides access to the underlying image data object.
        ImageData& getData() const;
    };
    
    std::vector<Gosu::Image> loadTiles(Graphics& graphics, const Bitmap& bmp, int tileWidth, int tileHeight, bool tileable);
    std::vector<Gosu::Image> loadTiles(Graphics& graphics, const std::wstring& bmp, int tileWidth, int tileHeight, bool tileable);
    
    //! Convenience function that splits a BMP or PNG file into an array
    //! of small rectangles and creates images from them.
    //! \param tileWidth If positive, specifies the width of one tile in
    //! pixels. If negative, the bitmap is divided into -tileWidth rows.
    //! \param tileHeight See tileWidth.
    //! \param appendTo STL container to which the images will be appended.
    //! Must provide a push_back member function; vector<tr1::shared_ptr<Image>>
    //! or boost::ptr_vector<Image> are good choices.
    template<typename Container>
    void imagesFromTiledBitmap(Graphics& graphics, const std::wstring& filename, int tileWidth, int tileHeight, bool tileable, Container& appendTo)
    {
        std::vector<Gosu::Image> tiles = loadTiles(graphics, filename, tileWidth, tileHeight, tileable);
        for (int i = 0, num = tiles.size(); i < num; ++i)
            appendTo.push_back(typename Container::value_type(new Gosu::Image(tiles[i])));
    }
    
    //! Convenience function that splits a bitmap into an area of array 
    //! rectangles and creates images from them.
    //! \param tileWidth If positive, specifies the width of one tile in
    //! pixels. If negative, the bitmap is divided into -tileWidth rows.
    //! \param tileHeight See tileWidth.
    //! \param appendTo STL container to which the images will be appended.
    //! Must provide a push_back member function; std::vector<std::tr1::shared_ptr<Image>>
    //! or boost::ptr_vector<Image> are good choices.
    template<typename Container>
    void imagesFromTiledBitmap(Graphics& graphics, const Bitmap& bmp,
        int tileWidth, int tileHeight, bool tileable, Container& appendTo)
    {
        std::vector<Gosu::Image> tiles = loadTiles(graphics, bmp, tileWidth, tileHeight, tileable);
        for (int i = 0, num = tiles.size(); i < num; ++i)
            appendTo.push_back(typename Container::value_type(new Gosu::Image(tiles[i])));
    }
}

#endif
