#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer.hpp>
#include <stdexcept>

#ifndef GOSU_IS_IPHONE

namespace 
{
    typedef std::vector<Gosu::Color> ColorTable;

    void decodePart(unsigned bitCount, Gosu::Bitmap& bmp, Gosu::Reader& reader,
        boost::uint32_t x, boost::uint32_t y, const ColorTable& colorTable)
    {
        if (bitCount == 24)
        {
            Gosu::Color color;
            color.setBlue(reader.getPod<boost::uint8_t>());
            color.setGreen(reader.getPod<boost::uint8_t>());
            color.setRed(reader.getPod<boost::uint8_t>());
            color.setAlpha(255);
            bmp.setPixel(x, y, color);
        }
        else if (bitCount == 8)
        {
            boost::uint8_t index;
            reader.readPod(index);
            if (index >= colorTable.size())
                throw std::runtime_error("Bitmap palette index out of range");
            bmp.setPixel(x, y, colorTable[index]);
        }
        else if (bitCount == 4)
        {
            boost::uint8_t doubleIndex, index[2];
            reader.readPod(doubleIndex);
            index[0] = (doubleIndex & 0xf0) >> 4;
            index[1] = (doubleIndex & 0x0f);
            
            for (int i = 0; i < 2; ++i)
            {
                if (index[i] > colorTable.size())
                    throw std::runtime_error("Bitmap palette index out of range");
                bmp.setPixel(x + i, y, colorTable[index[i]]);
            }
        }
    }
    
    template<unsigned bits>
    typename boost::uint_t<bits>::fast readVal(Gosu::Reader& reader)
    {
    	typename boost::uint_t<bits>::least val;
        reader.readPod(val, Gosu::boLittle);
	return val;
    }
}

Gosu::Reader Gosu::loadFromBMP(Bitmap& bmp, Reader reader)
{
    if (readVal<8>(reader) != 'B' || readVal<8>(reader) != 'M')
        throw std::runtime_error("File is not a bitmap!");

    // File header
    // Size
    readVal<32>(reader);
    // Reserved
    readVal<16>(reader);
    readVal<16>(reader);
    // Offset to bitmap data
    readVal<32>(reader);
    
    // Info header
    unsigned infoHeaderSize = readVal<32>(reader);
    if (infoHeaderSize != 12 && infoHeaderSize != 40)
        throw std::runtime_error("Unsupported BMP header");
    bool isOs2Bitmap = infoHeaderSize == 12;
    bool isTopDown = false;
    
    unsigned width, height, bitCount, clrUsed;
    if (isOs2Bitmap)
    {
        width = readVal<16>(reader);
        height = readVal<16>(reader);
        readVal<16>(reader); // planes
        bitCount = readVal<16>(reader);
        clrUsed = 1 << bitCount;
    }
    else
    {
        width = readVal<32>(reader);
        height = readVal<32>(reader);
        if (static_cast<int>(height) < 0)
        {
            isTopDown = true;
            height = -static_cast<int>(height);
        }
        readVal<16>(reader); // planes
        bitCount = readVal<16>(reader);
        readVal<32>(reader); // compression
        readVal<32>(reader); // image size
        readVal<32>(reader); // xPelsPerMeter
        readVal<32>(reader); // yPelsPerMeter
        clrUsed = readVal<32>(reader);
        readVal<32>(reader); // important colors
    }
    
    if (bitCount != 4 && bitCount != 8 && bitCount != 24)
        throw std::runtime_error("Unsupported BMP file bit depth");
    
    bmp.resize(width, height);
    
    ColorTable colors;
    if (bitCount != 24)
    {
        if (clrUsed == 0)
            clrUsed = 1 << bitCount;
        for (unsigned i = 0; i < clrUsed; ++i)
        {
            Gosu::Color color;
            color.setAlpha(255);
            color.setBlue(readVal<8>(reader));
            color.setGreen(readVal<8>(reader));
            color.setRed(readVal<8>(reader));
            if (!isOs2Bitmap)
                readVal<8>(reader); // reserved
            colors.push_back(color);
        }
    }
    
    int xOffset = 1;
    if (bitCount == 4)
        xOffset = 2;

    unsigned pixelBytesPerRow = 0;
    switch (bitCount)
    {
        case 4: pixelBytesPerRow = (width * 2 + 1) / 2; break;
        case 8: pixelBytesPerRow = width;
        case 24: pixelBytesPerRow = width * 3;
    }
    unsigned fillBytes = (4 - (pixelBytesPerRow % 4)) % 4;

    int begin = isTopDown ?      0 : height - 1;
    int end = isTopDown   ? height : -1;
    int step = isTopDown  ?     +1 : -1;
    
    if (bitCount == 24)
    {
        // Look-ahead-experiment.
        // Reducing Reader::read calls => performance on *nix?
        std::vector<boost::uint8_t> buf(height * (pixelBytesPerRow + fillBytes));
        boost::uint8_t* ptr = &buf[0];
        reader.read(ptr, buf.size());
        for (int y = begin; y != end; y += step)
        {
            for (unsigned x = 0; x < width; ++x)
            {
                Gosu::Color color;
                color.setBlue(*ptr++);
                color.setGreen(*ptr++);
                color.setRed(*ptr++);
                color.setAlpha(255);
                bmp.setPixel(x, y, color);
            }
            ptr += fillBytes;
        }
    }
    else
        for (int y = begin; y != end; y += step)
        {
            for (unsigned x = 0; x < width; x += xOffset)
                decodePart(bitCount, bmp, reader, x, y, colors);
            
            reader.seek(fillBytes);
        }
    
    return reader;
}

#endif

namespace
{
    template<unsigned bits, typename T>
    void writeVal(Gosu::Writer& writer, T value)
    {
        typename boost::uint_t<bits>::least val = value;
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

