#ifndef GOSUIMPL_GRAPHICS_TEXTFORMATTING
#define GOSUIMPL_GRAPHICS_TEXTFORMATTING

#include <Gosu/Color.hpp>
#include <Gosu/Utility.hpp>
#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <boost/variant.hpp>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cwchar>

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
        
        typedef std::pair<std::wstring, unsigned> SimpleImpl;
        typedef std::vector<FormattedChar> FancyImpl;
        
        boost::variant<SimpleImpl, FancyImpl> impl;
        
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
        
        explicit FormattedString(const std::wstring& html, unsigned baseFlags)
        {
            if (html.find_first_of(L"<&") == std::wstring::npos)
            {
                impl = SimpleImpl(html, baseFlags);
                return;
            }
            
            FancyImpl chars;
        
            unsigned pos = 0;
            int b = (baseFlags & ffBold) ? 1 : 0,
                u = (baseFlags & ffUnderline) ? 1 : 0,
                i = (baseFlags & ffItalic) ? 1 : 0;
            std::vector<Gosu::Color> c;
            c.push_back(0xffffffff);
            while (pos < html.length())
            {
                // TODO: range checking for the color ops so .at() doesn't crash valid strings
            
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
                if (html.length() >= pos + 10 &&
                    html.substr(pos, 3) == L"<c=" &&
                    html.at(pos + 9) == L'>')
                {
                    using namespace std;
                    boost::uint32_t rgb =
                        std::wcstoul(html.c_str() + pos + 3, 0, 16);
                    c.push_back(0xff000000 | rgb);
                    pos += 10;
                    continue;
                }
                if (html.length() >= pos + 12 &&
                    html.substr(pos, 3) == L"<c=" &&
                    html.at(pos + 11) == L'>')
                {
                    using namespace std;
                    boost::uint32_t argb =
                        std::wcstoul(html.c_str() + pos + 3, 0, 16);
                    c.push_back(argb);
                    pos += 12;
                    continue;
                }
                if (html.substr(pos, 4) == L"</c>")
                {
                    c.pop_back();
                    pos += 4;
                    continue;
                }
                if (html.substr(pos, 4) == L"&lt;")
                {
                    FormattedChar fc = { L'<', c.back(), flags(b,u,i) };
                    chars.push_back(fc);
                    pos += 4;
                    continue;
                }
                if (html.substr(pos, 4) == L"&gt;")
                {
                    FormattedChar fc = { L'>', c.back(), flags(b,u,i) };
                    chars.push_back(fc);
                    pos += 4;
                    continue;
                }
                if (html.substr(pos, 5) == L"&amp;")
                {
                    FormattedChar fc = { L'&', c.back(), flags(b,u,i) };
                    chars.push_back(fc);
                    pos += 5;
                    continue;
                }
                if (html[pos] == L'&' && pos < html.length() - 1)
                {
                    int endOfEntity = pos + 1;
                    while (html[endOfEntity] != L';')
                    {
                        using namespace std; // never know where the wchar_t stuff is ...
                        if (!iswalnum(static_cast<wint_t>(html[endOfEntity])))
                            goto normalCharacter;
                        endOfEntity += 1;
                        if (endOfEntity >= html.size())
                            goto normalCharacter;
                    }
                    FormattedChar fc = { 0, c.back(), 0, std::wstring(html.begin() + pos + 1, html.begin() + endOfEntity) };
                    if (!isEntity(fc.entity))
                        goto normalCharacter;
                    chars.push_back(fc);
                    pos = endOfEntity + 1;
                    continue;
                }
                
            normalCharacter:                
                FormattedChar fc = { html[pos], c.back(), flags(b,u,i) };
                chars.push_back(fc);
                pos += 1;
            }
            
            impl = chars;
        }
        
        std::wstring unformat() const
        {
            if (const SimpleImpl* simpl = boost::get<SimpleImpl>(&impl))
                return simpl->first;
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            std::wstring result(length(), L' ');
            for (int i = 0; i < chars.size(); ++i)
                result[i] = chars[i].wc;
            return result;
        }
        
        const wchar_t* entityAt(unsigned index) const
        {
            if (impl.which() == 0)
                return 0;
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            if (chars[index].wc != 0 || chars[index].entity.empty())
                return 0;
            return chars[index].entity.c_str();
        }
        
        wchar_t charAt(unsigned index) const
        {
            if (const SimpleImpl* simpl = boost::get<SimpleImpl>(&impl))
                return simpl->first[index];
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            return chars[index].wc;
        }
        
        unsigned flagsAt(unsigned index) const
        {
            if (const SimpleImpl* simpl = boost::get<SimpleImpl>(&impl))
                return simpl->second;
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            return chars[index].flags;
        }
        
        Gosu::Color colorAt(unsigned index) const
        {
            if (impl.which() == 0)
                return Color::WHITE;
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            return chars[index].color;
        }
        
        unsigned length() const
        {
            if (const SimpleImpl* simpl = boost::get<SimpleImpl>(&impl))
                return simpl->first.size();
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            return chars.size();
        }
        
        FormattedString range(unsigned begin, unsigned end) const
        {
            FormattedString result;

            if (const SimpleImpl* simpl = boost::get<SimpleImpl>(&impl))
                result.impl = SimpleImpl(std::wstring(simpl->first.begin() + begin, simpl->first.begin() + end), simpl->second);
            else
            {
                const FancyImpl& chars = boost::get<FancyImpl>(impl);
                result.impl = FancyImpl(chars.begin() + begin, chars.begin() + end);
            }
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
            if (impl.which() == 0)
                return std::vector<FormattedString>(1, *this);
            
            const FancyImpl& chars = boost::get<FancyImpl>(impl);
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 1; cur < length(); ++cur)
                if (!chars[begin].sameStyleAs(chars[cur]))
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
