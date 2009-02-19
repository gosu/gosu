#include <Gosu/Text.hpp>

// Just a stub so far

std::wstring Gosu::defaultFontName()
{
    // OF COURSE Helvetica is better - but the dots above my capital umlauts get
    // eaten when I use it with Gosu. Until this is fixed, keep Arial. (TODO)
    return L"Arial";
}

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    return text.length();
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
}
