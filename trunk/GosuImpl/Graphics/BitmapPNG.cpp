#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#if 0
#include <boost/thread.hpp>
#endif
#include <cassert>
#include <stdexcept>
#include <png.h>

namespace Gosu
{
    namespace
    {
#if 0
        // Stupid, but everyone would expect loadFromPNG/saveToPNG to be thread-safe.
        boost::mutex pngMutex;
#endif
        
        Reader* tmpReadStream;
        Writer* tmpWriteStream;

        void readPNGdata(png_structp png_ptr, png_bytep data, png_size_t length)
        {
            tmpReadStream->read(data, length);
        }
        void writePNGdata(png_structp png_ptr, png_bytep data, png_size_t length)
        {
            tmpWriteStream->write(data, length);
        }
        void flushPNGdata(png_structp png_ptr)
        {
        }
    }
}

Gosu::Reader Gosu::loadFromPNG(Bitmap& out, Reader reader)
{
#if 0
    boost::mutex::scoped_lock lock(pngMutex);
#endif

    Bitmap newBitmap;

    char header[8];
    reader.read(header, 8);
    if (png_sig_cmp(reinterpret_cast<png_byte*>(&header[0]), 0, 8))
        throw std::runtime_error("Invalid PNG file");

    png_structp pngPtr;
    // IMPR: Error checking.
    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!pngPtr)
        throw std::runtime_error("Can't create png_structp");

    png_infop infoPtr;
    infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_read_struct(&pngPtr, 0, 0);
        throw std::runtime_error("Can't create png_infop");
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        png_destroy_read_struct(&pngPtr, 0, 0);
        throw std::runtime_error("libpng error");
    }

    tmpReadStream = &reader;  // Highly experimental!
    png_set_read_fn(pngPtr, png_get_io_ptr(pngPtr), readPNGdata);

    png_set_sig_bytes(pngPtr, 8);

    png_read_png(pngPtr, infoPtr, PNG_TRANSFORM_PACKING |
        PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_SWAP_ALPHA, 0);

    // Get general information about the PNG.
    png_bytepp rows = png_get_rows(pngPtr, infoPtr);
    unsigned width = png_get_image_width(pngPtr, infoPtr),
        height = png_get_image_height(pngPtr, infoPtr),
        channels = png_get_channels(pngPtr, infoPtr),
        colorType = png_get_color_type(pngPtr, infoPtr),
        bitDepth = png_get_bit_depth(pngPtr, infoPtr);

    png_bytep trans;
    int numTrans = 0;
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
        png_get_tRNS(pngPtr, infoPtr, &trans, &numTrans, NULL);
    // This function only understands palette images with a bit depth <= 8 and
    // non-palette images with a bit depth of 8 (the latter is no problem
    // though, since the transformations given to png_read_png should change
    // all non-palette images to a bit depth of 8).
    if ((colorType == PNG_COLOR_TYPE_PALETTE && bitDepth > 8) ||
        (colorType != PNG_COLOR_TYPE_PALETTE && bitDepth != 8))
    {
        png_destroy_read_struct(&pngPtr, &infoPtr, 0);
        throw std::runtime_error("Unsupported bit depth");
    }

    // Get palette-related information about the PNG.
    unsigned palSize;
    png_colorp palColors;
    if (colorType == PNG_COLOR_TYPE_PALETTE)
    {
        if (!png_get_valid(pngPtr, infoPtr, PNG_INFO_PLTE))
        {
            png_destroy_read_struct(&pngPtr, &infoPtr, 0);
            throw std::runtime_error("PNG palette missing");
        }
        png_get_PLTE(pngPtr, infoPtr, &palColors,
            reinterpret_cast<int*>(&palSize));

        // This should be true after the bit-depth check above.
        assert(palSize <= 256);
    }

    try
    {
        newBitmap.resize(width, height);
    }
    catch (...)
    {
        png_destroy_read_struct(&pngPtr, &infoPtr, 0);
        throw;
    }

    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width; ++x)
        {
            Color c;

            switch (colorType)
            {
                case PNG_COLOR_TYPE_GRAY:
                {
                    c.setAlpha(255);
                    c.setRed  (rows[y][x * channels + 0]);
                    c.setGreen(rows[y][x * channels + 0]);
                    c.setBlue (rows[y][x * channels + 0]);
                    break;
                }
                case PNG_COLOR_TYPE_GRAY_ALPHA:
                {
                    c.setAlpha(rows[y][x * channels + 0]);
                    c.setRed  (rows[y][x * channels + 1]);
                    c.setGreen(rows[y][x * channels + 1]);
                    c.setBlue (rows[y][x * channels + 1]);
                    break;
                }
                case PNG_COLOR_TYPE_RGB:
                {
                    c.setAlpha(255);
                    c.setRed  (rows[y][x * channels + 0]);
                    c.setGreen(rows[y][x * channels + 1]);
                    c.setBlue (rows[y][x * channels + 2]);
                    break;
                }
                case PNG_COLOR_TYPE_RGB_ALPHA:
                {
                    c.setAlpha(rows[y][x * channels + 0]);
                    c.setRed  (rows[y][x * channels + 1]);
                    c.setGreen(rows[y][x * channels + 2]);
                    c.setBlue (rows[y][x * channels + 3]);
                    break;
                }
                case PNG_COLOR_TYPE_PALETTE:
                {
                    // First get the whole byte in which the palette index is
                    // stored (but possibly also adjacent indices).
                    png_byte palIndex = rows[y][x * bitDepth / 8];

                    // Shift the interesting index to the right-most bitDepth
                    // bits.
                    palIndex >>= 8 - (x % (8 / bitDepth) + 1) * bitDepth;

                    // Filter out the other indices.
                    palIndex &= (1 << bitDepth) - 1;

                    if (palIndex >= palSize)
                    {
                        png_destroy_read_struct(&pngPtr, &infoPtr, 0);
                        throw std::runtime_error("Palette index out of range");
                    }
                    
                    int alpha = 255;
                    for (int i = 0; i < numTrans; ++i)
                    {
                        if (trans[i] == palIndex)
                            alpha = 0;
                    }
                    c.setAlpha(alpha);
                    c.setRed  (palColors[palIndex].red);
                    c.setGreen(palColors[palIndex].green);
                    c.setBlue (palColors[palIndex].blue);
                    break;
                }
                default:
                {
                    png_destroy_read_struct(&pngPtr, &infoPtr, 0);
                    throw std::runtime_error("Unsupported PNG color type");
                }
            }

            // IMPR: Assumes non-throwing setPixel, which is not yet officially
            // guaranteed.
            newBitmap.setPixel(x, y, c);
        }

    // At last!
    png_destroy_read_struct(&pngPtr, &infoPtr, 0);

    newBitmap.swap(out);

    return reader; // TODO: Does this point to the right location?
}

Gosu::Writer Gosu::saveToPNG(const Bitmap& bmp, Writer writer)
{
#if 0
    boost::mutex::scoped_lock lock(pngMutex);
#endif

    png_structp pngPtr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr)
        throw std::runtime_error("Can't create png_structp");

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
        throw std::runtime_error("Can't create png_infop");
    }

    if (setjmp(png_jmpbuf(pngPtr)))        // error handling
    {
        png_destroy_write_struct(&pngPtr, &infoPtr);
        throw std::runtime_error("setjmp() failed");
    }

    tmpWriteStream = &writer;  // No more experimental.
    png_set_write_fn(pngPtr, png_get_io_ptr(pngPtr), writePNGdata, flushPNGdata);

    png_set_IHDR(pngPtr, infoPtr, bmp.width(), bmp.height(), 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(pngPtr, infoPtr);

    png_bytepp rows;
    rows = new png_bytep[bmp.height()];
    for(unsigned int y = 0; y < bmp.height(); y++)
    {
        rows[y] = new png_byte[bmp.width() * 3];
        for(unsigned int x = 0; x < bmp.width()*3; x += 3)
        {
            rows[y][x] = bmp.getPixel(x/3, y).red();
            rows[y][x+1] = bmp.getPixel(x/3, y).green();
            rows[y][x+2] = bmp.getPixel(x/3, y).blue();
        }
    }

    png_set_rows(pngPtr, infoPtr, rows);
    png_write_png(pngPtr, infoPtr, PNG_TRANSFORM_IDENTITY, NULL);

    for(unsigned int i = 0; i < bmp.height(); i++)
        delete[] rows[i];
    delete[] rows;

/*    png_write_image(pngPtr, row_pointers); // row_pointers == ?
    png_write_end(pngPtr, NULL);
    png_destroy_write_struct(&pngPtr, &infoPtr);*/
    // TODO: Hey, why is destroy_write_struct commented out?

    return writer; // TODO: Does this point to the right location?
}

