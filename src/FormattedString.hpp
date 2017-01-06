#pragma once

#include "GraphicsImpl.hpp"
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <cstdint>
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
            std::string entity;
            
            bool same_style_as(const FormattedChar& other) const
            {
                return wc && other.wc && color == other.color && flags == other.flags;
            }
        };
        
        // If characters.empty(), use these for the whole string.
        std::wstring simple_string;
        unsigned simple_flags;
        // If not characters.empty(), ignore above fields and use this.
        std::vector<FormattedChar> characters;
        
        static unsigned flags(int b, int u, int i)
        {
            unsigned flags = 0;
            if (b > 0) flags |= FF_BOLD;
            if (u > 0) flags |= FF_UNDERLINE;
            if (i > 0) flags |= FF_ITALIC;
            return flags;
        }
        
    public:
        FormattedString()
        {
        }
        
        explicit FormattedString(const wchar_t* html, unsigned base_flags)
        {
            // Remove \r characters if existent. Avoid a copy if we don't need one.
            std::wstring unixified;
            if (std::wcschr(html, L'\r')) {
                unixified.resize(std::wcslen(html));
                unsigned pos = 0;
                while (*html) {
                    if (*html != '\r') {
                        unixified[pos++] = *html;
                    }
                    ++html;
                }
                unixified.resize(pos);
                html = unixified.c_str();
            }
            
            std::size_t len = std::wcslen(html);
            
            // Just skip all this if there are entities or formatting tags in the string.
            if (std::wcscspn(html, L"<&") == len) {
                simple_string = html;
                simple_flags = base_flags;
                return;
            }
            
            unsigned pos = 0;
            int b = (base_flags & FF_BOLD) ? 1 : 0,
                u = (base_flags & FF_UNDERLINE) ? 1 : 0,
                i = (base_flags & FF_ITALIC) ? 1 : 0;
            std::vector<Gosu::Color> c;
            c.push_back(0xffffffff);
            while (pos < len) {
                if (!std::wcsncmp(html + pos, L"<b>", 3)) {
                    b += 1;
                    pos += 3;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</b>", 4)) {
                    b -= 1;
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<u>", 3)) {
                    u += 1;
                    pos += 3;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</u>", 4)) {
                    u -= 1;
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<i>", 3)) {
                    i += 1;
                    pos += 3;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</i>", 4)) {
                    i -= 1;
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<c=", 3) && len >= pos + 10
                    && html[pos + 9] == L'>') {
                    using namespace std;
                    unsigned rgb = static_cast<std::uint32_t>(wcstoul(html + pos + 3, 0, 16));
                    c.push_back(0xff000000 | rgb);
                    pos += 10;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"<c=", 3) && len >= pos + 12
                    && html[pos + 11] == L'>') {
                    using namespace std;
                    unsigned argb = static_cast<std::uint32_t>(wcstoul(html + pos + 3, 0, 16));
                    c.push_back(argb);
                    pos += 12;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"</c>", 4)) {
                    if (c.size() > 1) {
                        c.pop_back();
                    }
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"&lt;", 4)) {
                    FormattedChar fc = { L'<', c.back(), flags(b, u, i) };
                    characters.push_back(fc);
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"&gt;", 4)) {
                    FormattedChar fc = { L'>', c.back(), flags(b, u, i) };
                    characters.push_back(fc);
                    pos += 4;
                    continue;
                }
                if (!std::wcsncmp(html + pos, L"&amp;", 5)) {
                    FormattedChar fc = { L'&', c.back(), flags(b, u, i) };
                    characters.push_back(fc);
                    pos += 5;
                    continue;
                }
                if (html[pos] == L'&' && html[pos + 1]) {
                    int end_of_entity = pos + 1;
                    while (html[end_of_entity] != L';') {
                        using namespace std;
                        if (!iswalnum(static_cast<wint_t>(html[end_of_entity]))) {
                            goto normal_character;
                        }
                        end_of_entity += 1;
                        if (end_of_entity >= len) {
                            goto normal_character;
                        }
                    }
                    std::wstring entity(html + pos + 1, html + end_of_entity);
                    FormattedChar fc = { 0, c.back(), 0, wstring_to_utf8(entity) };
                    if (!is_entity(fc.entity)) {
                        goto normal_character;
                    }
                    characters.push_back(fc);
                    pos = end_of_entity + 1;
                    continue;
                }

            normal_character:
                FormattedChar fc = { html[pos], c.back(), flags(b, u, i) };
                characters.push_back(fc);
                pos += 1;
            }
        }
        
        std::wstring unformat() const
        {
            if (characters.empty()) {
                return simple_string;
            }
            
            std::wstring result(characters.size(), 0);
            for (int i = 0; i < characters.size(); ++i) {
                result[i] = characters[i].wc;
            }
            return result;
        }
        
        const char* entity_at(unsigned index) const
        {
            if (characters.empty()) {
                return nullptr;
            }
            
            if (characters[index].wc != 0 || characters[index].entity.empty()) {
                return nullptr;
            }
            
            return characters[index].entity.c_str();
        }
        
        wchar_t char_at(unsigned index) const
        {
            if (characters.empty()) {
                return simple_string[index];
            }
            else {
                return characters[index].wc;
            }
        }
        
        unsigned flags_at(unsigned index) const
        {
            if (characters.empty()) {
                return simple_flags;
            }
            else {
                return characters[index].flags;
            }
        }
        
        Gosu::Color color_at(unsigned index) const
        {
            if (characters.empty()) {
                return Color::WHITE;
            }
            else {
                return characters[index].color;
            }
        }
        
        std::size_t length() const
        {
            if (std::size_t len = characters.size()) {
                return len;
            }
            else {
                return simple_string.length();
            }
        }
        
        FormattedString range(std::size_t begin, std::size_t end) const
        {
            FormattedString result;
            if (characters.empty()) {
                result.simple_string.assign(simple_string.begin() + begin,
                                            simple_string.begin() + end);
                result.simple_flags = simple_flags;
            }
            else {
                result.characters.assign(characters.begin() + begin,
                                         characters.begin() + end);
            }
            return result;
        }
        
        std::vector<FormattedString> split_lines() const
        {
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 0; cur < length(); ++cur) {
                if (char_at(cur) == L'\n') {
                    result.push_back(range(begin, cur));
                    begin = cur + 1;
                }
            }
            result.push_back(range(begin, length()));
            return result;
        }
        
        std::vector<FormattedString> split_parts() const
        {
            if (characters.empty()) {
                return std::vector<FormattedString>(1, *this);
            }
            
            std::vector<FormattedString> result;
            unsigned begin = 0;
            for (unsigned cur = 1; cur < length(); ++cur) {
                if (!characters[begin].same_style_as(characters[cur])) {
                    result.push_back(range(begin, cur));
                    begin = cur;
                }
            }
            result.push_back(range(begin, length()));
            return result;
        }
    };
}
