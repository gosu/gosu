#include <Gosu/Platform.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
using namespace std;

wstring Gosu::defaultFontName()
{
    // OF COURSE Helvetica is better - but the dots above my capital umlauts get
    // eaten when I use it with Gosu. Until this is fixed, keep Arial. (TODO)
    return L"Arial";
}

unsigned Gosu::textWidth(const wstring& text,
    const wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    return text.length();
}

void Gosu::drawText(Bitmap& bitmap, const wstring& text, int x, int y,
    Color c, const wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
}
