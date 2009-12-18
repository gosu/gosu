#include <Gosu/Font.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <boost/array.hpp>
using namespace std;

namespace
{
    bool isRtlChar(wchar_t wc)
    {
        return wc == 0x202e;
    }
    
    bool isLtrChar(wchar_t wc)
    {
        return wc == 0x202d;
    }
    
    bool isFormattingChar(wchar_t wc)
    {
        return isLtrChar(wc) || isRtlChar(wc);
    }
}

struct Gosu::Font::Impl
{
    Graphics* graphics;
    wstring name;
    unsigned height;
    unsigned flags;

    // Chunk of 2^16 characters (on Windows, there'll only be one of them).
    // IMPR: I couldn't find a way to determine the size of wchar_t at compile
    // time, so I can't get rid of the magic numbers or even do some #ifdef
    // magic.
    typedef boost::array<boost::scoped_ptr<Image>, 65536> CharChunk;
    boost::scoped_ptr<CharChunk> chunks[65536];

    Image& getChar(wchar_t wc)
    {
        size_t chunkIndex = wc / 65536;
        size_t charIndex = wc % 65536;

        if (!chunks[chunkIndex])
            chunks[chunkIndex].reset(new CharChunk);

        boost::scoped_ptr<Image>& imgPtr = (*chunks[chunkIndex])[charIndex];

        if (imgPtr)
            return *imgPtr;

        wstring charString(1, wc);
        if (isFormattingChar(wc))
            charString.clear(); // Don't draw formatting characters
        unsigned charWidth = Gosu::textWidth(charString, name, height, flags);
        Bitmap bmp;
        bmp.resize(charWidth, height);
        
        drawText(bmp, charString, 0, 0, Color::WHITE, name, height, flags);
        imgPtr.reset(new Image(*graphics, bmp));
        return *imgPtr;
    }
};

Gosu::Font::Font(Graphics& graphics, const wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
: pimpl(new Impl)
{
    pimpl->graphics = &graphics;
    pimpl->name = fontName;
    pimpl->height = fontHeight * 2; // Auto-AA!
    pimpl->flags = fontFlags;
}

Gosu::Font::~Font()
{
}

std::wstring Gosu::Font::name() const
{
    return pimpl->name;
}

unsigned Gosu::Font::height() const
{
    return pimpl->height / 2;
}

unsigned Gosu::Font::flags() const
{
    return pimpl->flags;
}

double Gosu::Font::textWidth(const std::wstring& text, double factorX) const
{
    double result = 0;
    for (unsigned i = 0; i < text.length(); ++i)
        result += pimpl->getChar(text[i]).width();
    return result * factorX / 2;
}

void Gosu::Font::draw(const wstring& text, double x, double y, ZPos z,
    double factorX, double factorY, Color c, AlphaMode mode) const
{
    factorX /= 2;
    factorY /= 2;
    enum {
        LTR = 1,
        RTL = -1
    } dir = LTR;

    for (unsigned i = 0; i < text.length(); ++i)
    {
        if (isLtrChar(text[i]))
        {
            if (dir == RTL)
                x -= 2 * textWidth(text.substr(i + 1, wstring::npos)) * factorX, dir = LTR;
            continue;
        }
        if (isRtlChar(text[i]))
        {
            if (dir == LTR)
                x += 2 * textWidth(text.substr(i + 1, wstring::npos)) * factorX, dir = RTL;
            continue;
        }
    
        Image& curChar = pimpl->getChar(text[i]);
        if (dir == LTR)
            curChar.draw(x, y, z, factorX, factorY, c, mode);
        else
            curChar.draw(x - curChar.width() * factorX, y, z, factorX, factorY, c, mode);
        
        x += curChar.width() * factorX * dir;
    }
}
                     
void Gosu::Font::drawRel(const wstring& text, double x, double y, ZPos z,
    double relX, double relY, double factorX, double factorY, Color c,
    AlphaMode mode) const
{
    x -= textWidth(text) * factorX * relX;
    y -= height() * factorY * relY;
    
    draw(text, x, y, z, factorX, factorY, c, mode);
}

void Gosu::Font::drawRot(const wstring& text, double x, double y, ZPos z, double angle,
    double factorX, double factorY, Color c, AlphaMode mode) const
{
    factorX /= 2;
    factorY /= 2;
    
    double stepX = offsetX(angle + 90, 1.0), stepY = offsetY(angle + 90, 1.0);
    
    for (unsigned i = 0; i < text.length(); ++i)
    {
        Image& curChar = pimpl->getChar(text[i]);
        curChar.drawRot(x, y, z, angle, 0.0, 0.0, factorX, factorY, c, mode);
        x += curChar.width() * factorX * stepX;
        y += curChar.width() * factorX * stepY;
    }
}
