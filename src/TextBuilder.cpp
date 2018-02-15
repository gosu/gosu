#include "TextBuilder.hpp"
#include <Gosu/Text.hpp>
#include <cassert>
using namespace std;

Gosu::WordInfo::WordInfo(const string& font_name, int font_height, vector<FormattedString> parts)
{
    assert (! parts.empty());
    
    is_whitespace  = isspace((int) parts.front().string.front());
    is_end_of_line = parts.back().string.back() == '\n';
    // Remove the trailing backspace character to avoid errors from Gosu::text_width().
    if (is_end_of_line) parts.back().string.pop_back();
    
    width = 0;
    for (const auto& part : parts) {
        assert (! part.string.empty());
        
        width += text_width(part.string, font_name, font_height, part.flags);
    }
    
    this->parts = move(parts);
}

void Gosu::TextBuilder::flush_current_line(EndOfLineReason reason)
{
    if (current_line.empty()) {
        if (reason == END_OF_PARAGRAPH) allocate_next_line();
        return;
    }
    
    allocate_next_line();
    
    if (current_line.back().is_whitespace) current_line.pop_back();
    
    // Shouldn't happen because the first word on a line should never be whitespace.
    assert (! current_line.empty());
    
    double words_width = 0, whitespace_width = 0;
    for (const auto& word : current_line) {
        (word.is_whitespace ? whitespace_width : words_width) += word.width;
    }
    
    double x = 0;
    if (align == AL_RIGHT) {
        x = result.width() - words_width - whitespace_width;
    }
    else if (align == AL_CENTER) {
        x = (result.width() - words_width - whitespace_width) / 2.0;
    }
    
    double whitespace_factor = 1.0;
    if (align == AL_JUSTIFY && reason == LINE_TOO_LONG) {
        whitespace_factor = (result.width() - words_width) / whitespace_width;
    }
    
    int y = (used_lines - 1) * (font_height + line_spacing);
    
    for (const auto& word : current_line) {
        if (word.is_whitespace) {
            x += word.width * whitespace_factor;
        }
        else {
            for (const auto& part : word.parts) {
                draw_text(result, part.string, round(x), y, part.color,
                          font_name, font_height, part.flags);
            }
            x += word.width;
        }
    }
    
    current_line.clear();
    current_line_width = 0;
}

void Gosu::TextBuilder::allocate_next_line()
{
    if (used_lines == allocated_lines) {
        allocated_lines += 10;
        resize_to_allocated_lines();
    }
    
    ++used_lines;
}

void Gosu::TextBuilder::resize_to_allocated_lines()
{
    result.resize(result.width(),
                  font_height * allocated_lines + line_spacing * max(0, allocated_lines - 1));
}

Gosu::TextBuilder::TextBuilder(const string& font_name, int font_height, int line_spacing,
                               int width, Alignment align)
: font_name(font_name), font_height(font_height), line_spacing(line_spacing), align(align)
{
    // This class uses result.width() to remember its destination width, so store it in there.
    result.resize(width, 0);
}

Gosu::Bitmap Gosu::TextBuilder::move_into_bitmap()
{
    flush_current_line(END_OF_TEXT);
    
    // Shrink to fit the currently used height.
    allocated_lines = used_lines;
    resize_to_allocated_lines();
    return move(result);
}

void Gosu::TextBuilder::feed_word(vector<FormattedString>&& word)
{
    WordInfo new_word(font_name, font_height, word);
    
    if (current_line_width + new_word.width > result.width()) {
        // Can't fit it on the same line as before, so flush the last line before adding the word to
        // the next line.
        flush_current_line(LINE_TOO_LONG);
        
        if (new_word.is_whitespace) {
            // Do not wrap trailing whitespace onto the start of the next line - discard this word.
            return;
        }
    }
    
    current_line.emplace_back(new_word);
    current_line_width += new_word.width;

    if (current_line_width > result.width() || new_word.is_end_of_line) {
        flush_current_line(new_word.is_end_of_line ? END_OF_PARAGRAPH : LINE_TOO_LONG);
    }
}
