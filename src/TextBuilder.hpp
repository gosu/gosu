#pragma once

#include "GraphicsImpl.hpp"
#include "MarkupParser.hpp"
#include <vector>

namespace Gosu
{
    // A single word and its formatting context. A "word" is anything that is atomic in terms of
    // where Gosu can introduce line breaks. Even a single word has several "parts" because e.g. a
    // single character in the middle could be bold.
    struct WordInfo
    {
        std::vector<FormattedString> parts;
        bool is_whitespace;
        bool is_end_of_line;
        double width;

        WordInfo(const std::string& font_name, double font_height,
                 std::vector<FormattedString> parts);
    };

    class TextBuilder
    {
        // Parameters.
        std::string m_font_name;
        double m_font_height;
        double m_line_spacing;
        Alignment m_align;

        // Input.
        std::vector<WordInfo> m_current_line;
        int m_current_line_width = 0;
        enum EndOfLineReason : int;
        void flush_current_line(EndOfLineReason reason);

        // Output.
        Bitmap m_result;
        int m_used_lines = 0;
        int m_allocated_lines = 0;
        void allocate_next_line();
        void reallocate(int lines);

    public:
        TextBuilder(const std::string& font_name, int font_height, int line_spacing, int width,
                    Alignment align);

        void feed_word(std::vector<FormattedString> word);

        Bitmap move_into_bitmap() &&;
    };
}
