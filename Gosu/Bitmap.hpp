//! \file Bitmap.hpp
//! Interface of the Bitmap class.

#ifndef GOSU_BITMAP_HPP
#define GOSU_BITMAP_HPP

#include <Gosu/Color.hpp>
#include <Gosu/Fwd.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Platform.hpp>
#include <string>
#include <vector>

namespace Gosu
{
    //! Rectangular area of pixels, each represented by a Color value. Provides
    //! minimal drawing functionality and serves as a temporary holder for
    //! graphical resources which are usually turned into Images later.
    //! Has (expensive) value semantics.
    class Bitmap
    {
        unsigned w, h;
        std::vector<Color> pixels;

    public:
        Bitmap() : w(0), h(0) {}
        Bitmap(unsigned w, unsigned h, Color c = Color::NONE) : w(w), h(h), pixels(w * h, c) {}

        unsigned width()  const { return w; }
        unsigned height() const { return h; }

        void swap(Bitmap& other);

        void resize(unsigned width, unsigned height, Color c = Color::NONE);
        
        //! Returns the color at the specified position. x and y must be on the
        //! bitmap.
        Color getPixel(unsigned x, unsigned y) const { return pixels[y * w + x]; }

        //! Sets the pixel at the specified position to a color. x and y must
        //! be on the bitmap.
        void setPixel(unsigned x, unsigned y, Color c) { pixels[y * w + x] = c; }

        //! Inserts a bitmap at the given position. Parts of the inserted
        //! bitmap that would be outside of the target bitmap will be
        //! clipped away.
        void insert(const Bitmap& source, int x, int y);

        //! Inserts a portion of a bitmap at the given position. Parts of the
        //! inserted bitmap that would be outside of the target bitmap will be
        //! clipped away.
        void insert(const Bitmap& source, int x, int y, unsigned srcX,
            unsigned srcY, unsigned srcWidth, unsigned srcHeight);
		
		//! Direct access to the array of color values. May be useful for optimized
        //! OpenGL operations.
        const Color* data() const { return &pixels[0]; }
        Color* data()             { return &pixels[0]; }

        // Work with data() instead if you need fast operations.
        GOSU_DEPRECATED void fill(Color c);
        GOSU_DEPRECATED void replace(Color oldColor, Color newColor);
    };
    
    //! Loads any supported image into a Bitmap.
    void loadImageFile(Bitmap& bitmap, const std::wstring& filename);
    //! Loads any supported image into a Bitmap.
    void loadImageFile(Bitmap& bitmap, Reader input);
    
    //! Saves a Bitmap to a file.
    void saveImageFile(const Bitmap& bitmap, const std::wstring& filename);
    //! Saves a Bitmap to an arbitrary resource.
    void saveImageFile(const Bitmap& bitmap, Gosu::Writer writer,
        const std::wstring& formatHint = L"png");

    //! Set the alpha value of all pixels which are equal to the color key
    //! to zero. Color values are adjusted so that no borders show up when
    //! the image is stretched or rotated.
    void applyColorKey(Bitmap& bitmap, Color key);
    
    //! The reverse of applyColorKey. Resets all fully transparent pixels by
    //! a background color, makes all other pixels fully opaque.
    void unapplyColorKey(Bitmap& bitmap, Color background);
    
    void applyBorderFlags(Bitmap& dest, const Bitmap& source,
        unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
        unsigned borderFlags);	

    // Use loadImageFile/saveImageFile instead.
    GOSU_DEPRECATED Reader loadFromBMP(Bitmap& bmp, Reader reader);
    GOSU_DEPRECATED Writer saveToBMP(const Bitmap& bmp, Writer writer);
    GOSU_DEPRECATED Reader loadFromPNG(Bitmap& bmp, Reader reader);
    GOSU_DEPRECATED Writer saveToPNG(const Bitmap& bmp, Writer writer);
}

#endif
