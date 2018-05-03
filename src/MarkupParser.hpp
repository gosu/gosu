#pragma once

#include "GraphicsImpl.hpp"
#include <functional>
#include <vector>

namespace Gosu
{
    struct FormattedString
    {
        std::string string;
        Color color = Color::WHITE;
        unsigned flags = 0;
        
        bool can_be_merged_with(const FormattedString& other) const
        {
            return color == other.color && flags == other.flags;
        }
    };

    class MarkupParser
    {
        // The current parser position.
        const char* markup;
        // The start of the current substring range.
        const char* substring;

        // Current b/i/u counters. An opening tag increases by 1, a closing tag decreases by 1.
        // Text is bold/italic/underline when the respective counter is > 0.
        int b, i, u;

        // Current color stack.
        std::vector<Color> c{ Color::WHITE };

        enum { IGNORE_WORDS, ADDING_WORD, ADDING_WHITESPACE } word_state;
        std::vector<FormattedString> substrings;
        std::function<void (std::vector<FormattedString>)> consumer;

        unsigned flags() const;

        template<size_t N>
        bool match_and_skip(const char (&chars)[N])
        {
            return match_and_skip(chars, N - 1 /* do not match terminating zero */);
        }

        bool match_and_skip(const char* chars, std::size_t length);

        bool parse_markup();
        bool parse_escape_entity();

        void add_current_substring();
        void add_substring(std::string&& substring);
        void flush_to_consumer();

    public:
        MarkupParser(const char* markup, unsigned base_flags, bool split_words,
                     std::function<void (std::vector<FormattedString>)> consumer);
        void parse();
    };
}
