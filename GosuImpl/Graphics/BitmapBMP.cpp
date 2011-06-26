#include <Gosu/Platform.hpp>

// All other platforms use OS libraries instead.
#ifdef GOSU_IS_IPHONE
#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/TR1.hpp>

namespace
{
    template<unsigned bits> struct UintSelector;
    template<> struct UintSelector<8>  { typedef std::tr1::uint8_t  Type; };
    template<> struct UintSelector<16> { typedef std::tr1::uint16_t Type; };
    template<> struct UintSelector<32> { typedef std::tr1::uint32_t Type; };
    
    template<unsigned bits, typename T>
    void writeVal(Gosu::Writer& writer, T value)
    {
        typename UintSelector<bits>::Type val = value;
        writer.writePod(val, Gosu::boLittle);
    }
}

Gosu::Writer Gosu::saveToBMP(const Bitmap& bmp, Writer writer)
{
    // rowSize is the width ceiled to the next multiple of four.
    unsigned rowSize = bmp.width() * 3;
    rowSize = int((float(rowSize) - 1.0)/4.0 + 1.0) * 4;
    
    // File header
    // Type
    writeVal<8>(writer, 'B');
    writeVal<8>(writer, 'M');
    // Size
    writeVal<32>(writer, 14 + 40 + rowSize * bmp.height());
    // Reserved
    writeVal<16>(writer, 0);
    writeVal<16>(writer, 0);
    // Offset to data
    writeVal<32>(writer, 14 + 40);
        
    // Info header
    // Size
    writeVal<32>(writer, 40);
    // Width, height
    writeVal<32>(writer, bmp.width());
    writeVal<32>(writer, bmp.height());
    // Planes
    writeVal<16>(writer, 1);
    // Bit count
    writeVal<16>(writer, 24);
    // Compression
    writeVal<32>(writer, 0);
    // Image size
    writeVal<32>(writer, rowSize * bmp.height());
    // xPelsPerMeter/yPelsPerMeter
    writeVal<32>(writer, 0);
    writeVal<32>(writer, 0);
    // Used and important colors
    writeVal<32>(writer, 0);
    writeVal<32>(writer, 0);
        
    for (int y = bmp.height() - 1; y >= 0; y--)
    {
        for (unsigned x = 0; x < bmp.width(); x++)
        {
            // BGR order
            writeVal<8>(writer, bmp.getPixel(x, y).blue());
            writeVal<8>(writer, bmp.getPixel(x, y).green());
            writeVal<8>(writer, bmp.getPixel(x, y).red());
        }
        int offset = rowSize - bmp.width() * 3;
        for(int i = 0; i < offset; ++i)
            writeVal<8>(writer, 0);
    }
    return writer;
}

#endif
