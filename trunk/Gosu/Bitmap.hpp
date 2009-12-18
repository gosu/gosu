//! \file Bitmap.hpp
//! Interface of the Bitmap class.

#ifndef GOSU_BITMAP_HPP
#define GOSU_BITMAP_HPP

#include <Gosu/Color.hpp>
#include <Gosu/Fwd.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <boost/scoped_ptr.hpp>
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
        Bitmap();

        unsigned width() const { return w; }
        unsigned height() const { return h; }

        void swap(Bitmap& other);

        void resize(unsigned width, unsigned height, Color c = Color::NONE);

        void fill(Color c);
        void replace(Color oldColor, Color newColor);

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
        const unsigned* data() const { return reinterpret_cast<const unsigned*>(&pixels[0]); }
        unsigned* data() { return reinterpret_cast<unsigned*>(&pixels[0]); }
    };

    //! Loads a Windows or OS/2 BMP file into the given bitmap.
    Reader loadFromBMP(Bitmap& bmp, Reader reader);
    //! Saves the contents of the given bitmap into windows BMP file data.
    Writer saveToBMP(const Bitmap& bmp, Writer writer);
    //! Loads a PNG file into the given bitmap.
    Reader loadFromPNG(Bitmap& bmp, Reader reader);
    //! Saves the contents of the given bitmap into PNG file data, 24 bits.
    Writer saveToPNG(const Bitmap& bmp, Writer writer);

    //! Set the alpha value of all pixels which are equal to the color key
    //! to zero. Color values are adjusted so that no borders show up when
    //! the image is stretched or rotated.
    void applyColorKey(Bitmap& bitmap, Color key);

    void applyBorderFlags(Bitmap& dest, const Bitmap& source,
        unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
        unsigned borderFlags);	

	// Still to be moved around & undocumented, beware! (TODO)
	Bitmap quickLoadBitmap(const std::wstring& filename);
}

#endif
