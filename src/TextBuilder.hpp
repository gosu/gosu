#pragma once

#include "GraphicsImpl.hpp"
#include "MarkupParser.hpp"
#include <vector>

namespace Gosu
{
    struct FormattedString;
    
    struct WordInfo
    {
        std::vector<FormattedString> parts;
        bool is_whitespace;
        bool is_end_of_line;
        int width;
        
        WordInfo(const std::string& font_name, int font_height, std::vector<FormattedString> parts);
    };
    
    class TextBuilder
    {
        // Parameters.
        std::string font_name;
        int font_height;
        int line_spacing;
        Alignment align;
        
        enum EndOfLineReason {
            LINE_TOO_LONG,
            END_OF_PARAGRAPH,
            END_OF_TEXT
        };
        
        // Input.
        std::vector<WordInfo> current_line;
        int current_line_width = 0;
        void flush_current_line(EndOfLineReason reason);

        // Output.
        Bitmap result;
        int used_lines = 0;
        int allocated_lines = 0;
        void allocate_next_line();
        void resize_to_allocated_lines();
        
    public:
        TextBuilder(const std::string& font_name, int font_height, int line_spacing,
                    int width, Alignment align);
        
        void feed_word(std::vector<FormattedString>&& word);

        Bitmap move_into_bitmap();
    };
}

