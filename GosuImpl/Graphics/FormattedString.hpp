#ifndef GOSUIMPL_GRAPHICS_TEXTFORMATTING
#define GOSUIMPL_GRAPHICS_TEXTFORMATTING

#include <Gosu/Color.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/TR1.hpp>
#include <cassert>
#include <cwchar>
#include <cwctype>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Gosu
{
    class FormattedString
    {
        struct FormattedChar
        {
            wchar_t wc;
            Gosu::Color color;
            unsigned flags;
            std::wstring entity;
            
            bool sameStyleAs(const FormattedChar& other) const
            {
                return wc && other.wc && color == other.color && flags == other.flags;
            }
        };
        
        // If characters.empty(), use these for the whole string.
        std::wstring simpleString;
        unsigned simpleFlags;
        // If not characters.empty(), ignore above fields and use this.
        std::vector<FormattedChar> characters;
        
        static unsigned flags(int b, int u, int i)
        {
            unsigned flags = 0;
            if (b > 0) flags |= ffBold;
            if (u > 0) flags |= ffUnderline;
            if (i > 0) flags |= ffItalic;
            return flags;
        }
        
    public:
        FormattedString()
        {
        }
        
        explicit FormattedString(const wchar_t* html, unsigned baseFlags)
        {
            // Remove \r characters if existent. Avoid a copy if we don't need one.
            std::wstring unixified;
            if (std::wcschr(html, L'\r')) {
                unixified.resize(std::wcslen(html));
                unsigned pos = 0;
                while (*html)
                {
                    if (*html != '\r')
                        unixified[pos++] = *html;
                    ++html;
                }
                unixified.resize(pos);
                html = unixified.c_str();
            }
            
            std::size_t len = std::wcslen(html);
            
            // Just skip all this if there are entities or formatting tags in the string.
            if (std::wcscspn(html, L"<&") == len)
            {
                simpleString = html;
                simpleFlags = baseFlags;
                return;
            }
            
            unsigned pos = 0;
            int b = (baseFlags & ffBold) ? 1 : 0,
                u = (baseFlags & ffUnderline) ? 1 : 0,
                i = (baseFlags & ffItalic) ? 1 : 0;
            std::vector<Gosu::Color> c;
            c.push_back(0xffffffff);
            while (pos < len)
            {
                if (!std::wcsncmp(html + pos, L"<b>", 3))
                {
                    b += 1;
                    pos += 3;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</b>", 4))
                {
                    b -= 1;
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<u>", 3))
                {
                    u += 1;
                    pos += 3;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</u>", 4))
                {
                    u -= 1;
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<i>", 3))
                {
                    i += 1;
                    pos += 3;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</i>", 4))
                {
                    i -= 1;
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<c=", 3) &&
                    len >= pos + 10 && html[pos + 9] == L'>')
                {
                    using namespace std;
                    unsigned rgb = wcstoul(html + pos + 3, 0, 16);
                    c.push_back(0xff000000 | rgb);
                    pos += 10;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<c=", 3) &&
                    len >= pos + 12 && html[pos + 11] == L'>')
                {
                    using namespace std;
                    unsigned argb = wcstoul(html + pos + 3, 0, 16);
                    c.push_back(argb);
                    pos += 12;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</c>", 4))
                {
                    if (c.size() > 1)
                        c.pop_back();
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"&lt;", 4))
                {
                    FormattedChar fc = { L'<', c.back(), flags(b,u,i) };
                    characters.push_back(fc);
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"&gt;", 4))
                {
                    FormattedChar fc = { L'>', c.back(), flags(b,u,i) };
                    characters.push_back(fc);
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"&amp;", 5))
                {
                    FormattedChar fc = { L'&', c.back(), flags(b,u,i) };
                    characters.push_back(fc);
                    pos += 5;
                    continue;
                }
                if (html[pos] == L'&' && html[pos + 1])
                {
                    int endOfEntity = pos + 1;
                    while (html[endOfEntity] != L';')
                    {
                        // Never know where the wchar_t stuff is...what a mess!
                        using namespace std;
                        using namespace std::tr1;
                        if (!iswalnum(static_cast<wint_t>(html[endOfEntity])))
                            goto normalCharacter;
                        endOfEntity += 1;
                        if (endOfEntity >= len)
                            goto normalCharacter;
                    }
                    FormattedChar fc = { 0, c.back(), 0, std::wstring(html + pos + 1, html + endOfEntity) };
                    if (!isEntity(fc.entity))
                        goto normalCharacter;
                    characters.push_back(fc);
                    pos = endOfEntity + 1;
                    continue;
                }
                
            normalCharacter:                
                FormattedChar fc = { html[pos], c.back(), flags(b,u,i) };
                characters.push_back(fc);
                pos += 1;
            }
        }
        
        std::wstring unformat() const
        {
            if (characters.empty())
                return simpleString;
            
            std::wstring result(characters.size(), 0);
            for (int i = 0; i < characters.size(); ++i)
                result[i] = characters[i].wc;
            return result;
        }
        
        const wchar_t* entityAt(unsigned index) const
        {
            if (characters.empty())
                return 0;
            
            assert (index <= characters.size());
            
            if (characters[index].wc != 0 || characters[index].entity.empty())
                return 0;
            return characters[index].entity.c_str();
        }
        
        wchar_t charAt(unsigned index) const
        {
            if (characters.empty())
                return simpleString[index];
            else
                return characters[index].wc;
        }
        
        unsigned flagsAt(unsigned index) const
        {
            if (characters.empty())
                return simpleFlags;
            else
                return characters[index].flags;
        }
        
        Gosu::Color colorAt(unsigned index) const
        {
            if (characters.empty())
                return Color::WHITE;
            else
                return characters[index].color;
        }
        
        unsigned length() const
        {
            if (unsigned len = characters.size())
                return len;
            else
                return simpleString.length();
        }
        
        FormattedString range(unsigned begin, unsigned end) const
        {
            FormattedString result;
            if (characters.empty())
            {
                result.simpleString.assign(simpleString.begin() + begin, simpleString.begin() + end);
                result.simpleFlags = simpleFlags;
            }
            else
                result.characters.assign(characters.begin() + begin, characters.begin() + end);
            return result;
        }
        
        std::vector<FormattedString> splitLines() const
        {
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 0; cur < length(); ++cur)
                if (charAt(cur) == L'\n')
                {
                    result.push_back(range(begin, cur));
                    begin = cur + 1;
                }
            result.push_back(range(begin, length()));
            return result;
        }
        
        std::vector<FormattedString> splitParts() const
        {
            if (characters.empty())
                return std::vector<FormattedString>(1, *this);
            
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 1; cur < length(); ++cur)
                if (!characters[begin].sameStyleAs(characters[cur]))
                {
                    result.push_back(range(begin, cur));
                    begin = cur;
                }
            result.push_back(range(begin, length()));
            return result;
        }
    };
}

#endif
