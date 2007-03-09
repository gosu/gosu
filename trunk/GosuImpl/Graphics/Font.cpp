#include <Gosu/Font.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <boost/array.hpp>
using namespace std;

struct Gosu::Font::Impl
{
    Graphics* graphics;
    wstring fontName;
    unsigned height;

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
        unsigned charWidth = Gosu::textWidth(charString, fontName, height, ffBold);
        Bitmap bmp;
        bmp.resize(charWidth, height);
        
        drawText(bmp, charString, 0, 0, Colors::white, fontName, height,
            ffBold);
        imgPtr.reset(new Image(*graphics, bmp));
        return *imgPtr;
    }
};

Gosu::Font::Font(Graphics& graphics, const wstring& fontName, unsigned height)
: pimpl(new Impl)
{
    pimpl->graphics = &graphics;
    pimpl->fontName = fontName;
    pimpl->height = height;
}

Gosu::Font::~Font()
{
}

unsigned Gosu::Font::height() const
{
    return pimpl->height;
}

double Gosu::Font::textWidth(const std::wstring& text, double factorX) const
{
    double result = 0;
    for (unsigned i = 0; i < text.length(); ++i)
        result += pimpl->getChar(text[i]).width() * factorX;
    return result;
}

void Gosu::Font::draw(const wstring& text, double x, double y, ZPos z,
    double factorX, double factorY, Color c, AlphaMode mode) const
{
    for (unsigned i = 0; i < text.length(); ++i)
    {
        Image& curChar = pimpl->getChar(text[i]);
        curChar.draw(x, y, z,
            factorX, factorY, c, mode);
        x += curChar.width() * factorX;
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
