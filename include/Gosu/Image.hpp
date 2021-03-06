//! \file Image.hpp
//! Interface of the Image class and helper functions.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Gosu
{
    //! Provides functionality for drawing rectangular images.
    class Image
    {
        std::shared_ptr<ImageData> data_;

    public:
        //! Creates an empty image. It will have a width and height of 0, and not contain anything.
        Image();
        
        //! Loads an image from a given filename.
        //!
        //! A color key of #ff00ff is automatically applied to BMP image files.
        //! For more flexibility, use the corresponding constructor that uses a Bitmap object.
        explicit Image(const std::string& filename, unsigned image_flags = IF_SMOOTH);
        
        //! Loads a portion of the the image at the given filename..
        //!
        //! A color key of #ff00ff is automatically applied to BMP image files.
        //! For more flexibility, use the corresponding constructor that uses a Bitmap object.
        Image(const std::string& filename, int src_x, int src_y,
            int src_width, int src_height, unsigned image_flags = IF_SMOOTH);
        
        //! Converts the given bitmap into an image.
        explicit Image(const Bitmap& source, unsigned image_flags = IF_SMOOTH);
        
        //! Converts a portion of the given bitmap into an image.
        Image(const Bitmap& source, int src_x, int src_y, int src_width,
              int src_height, unsigned image_flags = IF_SMOOTH);
        
        //! Creates an Image from a user-supplied instance of the ImageData interface.
        explicit Image(std::unique_ptr<ImageData>&& data);

        unsigned width() const;
        unsigned height() const;

        //! Draws the image so its upper left corner is at (x; y).
        void draw(double x, double y, ZPos z = 0, double scale_x = 1, double scale_y = 1,
            Color c = Color::WHITE, BlendMode mode = BM_DEFAULT) const;
        //! Like draw(), but with modulation colors for all four corners.
        void draw_mod(double x, double y, ZPos z, double scale_x, double scale_y,
            Color c1, Color c2, Color c3, Color c4, BlendMode mode = BM_DEFAULT) const;

        //! Draws the image rotated by the given angle so that its rotation
        //! center is at (x; y). Note that this is different from how all the
        //! other drawing functions work!
        //! \param angle See Math.hpp for an explanation of how Gosu interprets
        //! angles.
        //! \param center_x Relative horizontal position of the rotation center
        //! on the image. 0 is the left border, 1 is the right border, 0.5 is
        //! the center (and default).
        //! \param center_y See center_x.
        void draw_rot(double x, double y, ZPos z = 0, double angle = 0,
            double center_x = 0.5, double center_y = 0.5, double scale_x = 1, double scale_y = 1,
            Color c = Color::WHITE, BlendMode mode = BM_DEFAULT) const;
        
        #ifndef SWIG
        //! Provides access to the underlying image data object.
        ImageData& data() const;
        #endif
    };
    
    #ifndef SWIG
    //! Convenience function that slices an image file into a grid and creates images from them.
    //! \param tile_width If positive, specifies the width of one tile in pixels.
    //! If negative, the bitmap is divided into -tile_width rows.
    //! \param tile_height See tile_width.
    std::vector<Gosu::Image> load_tiles(const Bitmap& bmp,
        int tile_width, int tile_height, unsigned image_flags = IF_SMOOTH);
    
    //! Convenience function that slices a bitmap into a grid and creates images from them.
    //! \param tile_width If positive, specifies the width of one tile in pixels.
    //! If negative, the bitmap is divided into -tile_width rows.
    //! \param tile_height See tile_width.
    std::vector<Gosu::Image> load_tiles(const std::string& filename,
        int tile_width, int tile_height, unsigned image_flags = IF_SMOOTH);
    #endif
}
