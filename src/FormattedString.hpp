#pragma once

#include "GraphicsImpl.hpp"
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <vector>

namespace Gosu
{
    class MarkupParser;

    struct FormattedSubstring
    {
        // If entity is empty, then there must be a non-empty substring, and vice versa.
        // If we encoded both an entity and a string in the same FormattedSubstring object, it would
        // be hard to tell in which order they should be rendered.
        
        std::string entity;
        std::string string;
        Color color;
        unsigned flags;
        
        bool can_be_merged_with(const FormattedSubstring& other) const
        {
            // Substrings that consist of an entity are never compatible.
            if (! string.empty() || ! other.string.empty()) return false;

            return color == other.color && flags == other.flags;
        }
    };

    // A FormattedString is a string composed of several (or zero) substrings that can each have a
    // distinct color or font flags.
    class FormattedString
    {
        std::vector<FormattedSubstring> substrings;
        
    public:
        FormattedString();
        FormattedString(const char* markup, unsigned base_flags);
    };
}

