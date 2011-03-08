#include <Gosu/Font.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/FormattedString.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
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
    unsigned height, flags;

    // Chunk of 2^16 characters (on Windows, there'll only be one of them).
    // IMPR: I couldn't find a way to determine the size of wchar_t at compile
    // time, so I can't get rid of the magic numbers or even do some #ifdef
    // magic.
    typedef boost::array<boost::scoped_ptr<Image>, 65536> CharChunk;
    boost::scoped_ptr<CharChunk> chunks[65536][ffCombinations];
    
    std::map<std::wstring, boost::shared_ptr<Image> > entityCache;

    const Image& imageAt(const FormattedString& fs, unsigned i)
    {
        if (fs.entityAt(i))
        {
            boost::shared_ptr<Image>& ptr = entityCache[fs.entityAt(i)];
            if (!ptr)
                ptr.reset(new Image(*graphics, entityBitmap(fs.entityAt(i)), false));
            return *ptr;
        }
        
        wchar_t wc = fs.charAt(i);
        unsigned flags = fs.flagsAt(i);
        
        assert(flags < ffCombinations);
    
        size_t chunkIndex = wc / 65536;
        size_t charIndex = wc % 65536;

        if (!chunks[chunkIndex][flags])
            chunks[chunkIndex][flags].reset(new CharChunk);

        boost::scoped_ptr<Image>& imgPtr = (*chunks[chunkIndex][flags])[charIndex];

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
    
    double factorAt(const FormattedString& fs, unsigned index) const
    {
        if (fs.entityAt(index))
            return 1;
        else
            return 0.5;
    }
};

Gosu::Font::Font(Graphics& graphics, const wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
: pimpl(new Impl)
{
    pimpl->graphics = &graphics;
    pimpl->name = fontName;
    pimpl->height = fontHeight * 2;
    pimpl->flags = fontFlags;
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
    FormattedString fs(text, flags());
    double result = 0;
    for (unsigned i = 0; i < fs.length(); ++i)
        result += pimpl->imageAt(fs, i).width() * pimpl->factorAt(fs, i);
    return result * factorX;
}

void Gosu::Font::draw(const wstring& text, double x, double y, ZPos z,
    double factorX, double factorY, Color c, AlphaMode mode) const
{
    FormattedString fs(text, flags());

    enum {
        LTR = 1,
        RTL = -1
    } dir = LTR;

    for (unsigned i = 0; i < fs.length(); ++i)
    {
        /*
        
        Sorry, LTR/RTL support taken out until somebody uses it
        
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
        }*/
        
        const Image& curChar = pimpl->imageAt(fs, i);
        double curFactor;
        Gosu::Color color;
        if (fs.entityAt(i))
            curFactor = 1.0, color = Gosu::Color(fs.colorAt(i).alpha() * c.alpha() / 255, 255, 255, 255);
        else
            curFactor = 0.5, color = Gosu::multiply(fs.colorAt(i), c);

        //if (dir == LTR)
            curChar.draw(x, y, z, factorX * curFactor, factorY * curFactor, color, mode);
        //else
        //    curChar.draw(x - curChar.width() * factorX, y, z, factorX, factorY, c, mode);
        
        x += curChar.width() * factorX * curFactor * dir;
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
    pimpl->graphics->pushTransform(rotate(angle, x, y));
    draw(text, x, y, z, factorX, factorY, c, mode);
    pimpl->graphics->popTransform();
}
