#pragma once

#include "GraphicsImpl.hpp"
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <vector>

namespace Gosu
{
    class FormattedString
    {
        struct FormattedChar
        {
            wchar_t wc;
            Color color;
            unsigned flags;
            std::string entity;
            
            bool same_style_as(const FormattedChar& other) const;
        };
        
        // If characters.empty(), use these for the whole string.
        std::wstring simple_string;
        unsigned simple_flags;
        // If not characters.empty(), ignore above fields and use this.
        std::vector<FormattedChar> characters;
        
    public:
        FormattedString();
        FormattedString(const wchar_t* html, unsigned base_flags);
        
        std::wstring unformat() const;
        
        const char* entity_at(unsigned index) const;
        wchar_t char_at(unsigned index) const;
        unsigned flags_at(unsigned index) const;
        Color color_at(unsigned index) const;
        
        std::size_t length() const;
        
        FormattedString range(std::size_t begin, std::size_t end) const;
        
        std::vector<FormattedString> split_lines() const;
        std::vector<FormattedString> split_parts() const;
    };
}

