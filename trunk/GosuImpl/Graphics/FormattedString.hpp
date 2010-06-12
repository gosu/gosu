#ifndef GOSUIMPL_GRAPHICS_TEXTFORMATTING
#define GOSUIMPL_GRAPHICS_TEXTFORMATTING

#include <Gosu/Color.hpp>
#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
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
            
            bool sameStyleAs(const FormattedChar& other) const
            {
                return color == other.color && flags == other.flags;
            }
        };
        std::vector<FormattedChar> chars;
        
    public:
        explicit FormattedString(const std::wstring& html)
        {
            unsigned pos = 0;
            int b = 0, u = 0, i = 0;
            std::vector<Gosu::Color> c;
            c.push_back(0xffffffff);
            while (pos < html.length())
            {
                // TODO: Range checking for the color ops
            
                if (html.substr(pos, 3) == L"<b>")
                {
                    b += 1;
                    pos += 3;
                    continue;
                }
                if (html.substr(pos, 4) == L"</b>")
                {
                    b -= 1;
                    pos += 4;
                    continue;
                }
                if (html.substr(pos, 3) == L"<u>")
                {
                    u += 1;
                    pos += 3;
                    continue;
                }
                if (html.substr(pos, 4) == L"</u>")
                {
                    u -= 1;
                    pos += 4;
                    continue;
                }
                if (html.substr(pos, 3) == L"<i>")
                {
                    i += 1;
                    pos += 3;
                    continue;
                }
                if (html.substr(pos, 4) == L"</i>")
                {
                    i -= 1;
                    pos += 4;
                    continue;
                }
                if (html.substr(pos, 3) == L"<c=" &&
                    html.at(pos + 9) == L'>')
                {
                    using namespace std;
                    boost::uint32_t rgb =
                        std::wcstol(html.c_str() + pos + 3, 0, 16);
                    c.push_back(0xff000000 | rgb);
                    pos += 10;
                }
                if (html.substr(pos, 3) == L"<c=" &&
                    html.at(pos + 11) == L'>')
                {
                    using namespace std;
                    boost::uint32_t argb =
                        std::wcstoll(html.c_str() + pos + 3, 0, 16);
                    c.push_back(argb);
                    pos += 12;
                }
                if (html.substr(pos, 4) == L"</c>")
                {
                    c.pop_back();
                    pos += 4;
                }
                
                unsigned flags = 0;
                if (b > 0) flags |= ffBold;
                if (u > 0) flags |= ffUnderline;
                if (i > 0) flags |= ffItalic;
                FormattedChar fc = { html[pos], c.back(), flags };
                chars.push_back(fc);
                pos += 1;
            }
        }
        
        std::wstring unformat() const
        {
            std::wstring result(length(), L' ');
            for (int i = 0; i < chars.size(); ++i)
                result[i] = chars[i].wc;
            return result;
        }
        
        wchar_t charAt(unsigned index) const
        {
            return chars[index].wc;
        }
        
        unsigned flagsAt(unsigned index) const
        {
            return chars[index].flags;
        }
        
        Gosu::Color colorAt(unsigned index) const
        {
            return chars[index].color;
        }
        
        unsigned length() const
        {
            return chars.size();
        }
        
        std::vector<FormattedString> splitLines() const
        {
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 0; cur < length(); ++cur)
            {
                if (charAt(cur) == L'\n')
                {
                    FormattedString line(L"");
                    line.chars.assign(chars.begin() + begin, chars.begin() + cur);
                    result.push_back(line);
                    begin = cur + 1;
                }
            }
            FormattedString line(L"");
            line.chars.assign(chars.begin() + begin, chars.end());
            result.push_back(line);
            return result;
        }
        
        std::vector<FormattedString> splitParts() const
        {
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 1; cur < length(); ++cur)
            {
                if (!chars[begin].sameStyleAs(chars[cur]))
                {
                    FormattedString line(L"");
                    line.chars.assign(chars.begin() + begin, chars.begin() + cur);
                    result.push_back(line);
                    begin = cur;
                }
            }
            FormattedString line(L"");
            line.chars.assign(chars.begin() + begin, chars.end());
            result.push_back(line);
            return result;
        }
    };
}

#endif
