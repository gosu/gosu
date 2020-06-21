#include "TextBuilder.hpp"
#include <Gosu/Text.hpp>

#include <utf8proc.h>

#include <cassert>
#include <cmath>
using namespace std;

Gosu::WordInfo::WordInfo(const string& font_name, double font_height, vector<FormattedString> parts)
{
    assert (!parts.empty());
    
    auto* properties = utf8proc_get_property(parts.front().text.front());

    // Also check the BiDi class to filter out non-breaking spaces.
    is_whitespace = properties->category == UTF8PROC_CATEGORY_ZS &&
                    properties->bidi_class == UTF8PROC_BIDI_CLASS_WS;

    is_end_of_line = parts.back().text.back() == '\n';
    // Remove the trailing backspace character to avoid errors from Gosu::text_width().
    if (is_end_of_line) parts.back().text.pop_back();
    
    width = 0;
    for (const auto& part : parts) {
        assert (is_end_of_line || !part.text.empty());
        
        width += text_width(part.text, font_name, font_height, part.flags);
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
    
    // Remove trailing whitespace so that justifying the text across the line works.
    if (current_line.back().is_whitespace) current_line.pop_back();
    
    // Shouldn't happen because the first word on a line should never be whitespace.
    assert (!current_line.empty());
    
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
    if (align == AL_JUSTIFY && whitespace_width != 0 && reason == LINE_TOO_LONG) {
        whitespace_factor = (result.width() - words_width) / whitespace_width;
    }
    
    double y = (used_lines - 1) * (font_height + line_spacing);
    
    for (const auto& word : current_line) {
        if (word.is_whitespace) {
            x += word.width * whitespace_factor;
        }
        else {
            for (const auto& part : word.parts) {
                draw_text(result, x, y, part.color, part.text, font_name, font_height, part.flags);
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
    double new_height = font_height * allocated_lines + line_spacing * max(0, allocated_lines - 1);
    result.resize(result.width(), ceil(new_height));
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

    if (new_word.is_end_of_line) {
        flush_current_line(END_OF_PARAGRAPH);
    }
}
