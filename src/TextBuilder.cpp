#include "TextBuilder.hpp"
#include <Gosu/Text.hpp>
#include <cassert>
#include <cmath>
#include <utf8proc.h>

Gosu::WordInfo::WordInfo(const std::string& font_name, double font_height,
                         std::vector<FormattedString> parts)
{
    assert(!parts.empty());

    const auto* properties = utf8proc_get_property(parts.front().text.front());

    // Also check the BiDi class to filter out non-breaking spaces.
    is_whitespace = properties->category == UTF8PROC_CATEGORY_ZS &&
                    properties->bidi_class == UTF8PROC_BIDI_CLASS_WS;

    is_end_of_line = parts.back().text.back() == '\n';
    // Remove the trailing backspace character to avoid errors from Gosu::text_width().
    if (is_end_of_line) parts.back().text.pop_back();

    width = 0;
    for (const auto& part : parts) {
        assert(is_end_of_line || !part.text.empty());

        width += text_width(part.text, font_name, font_height, part.flags);
    }

    this->parts = move(parts);
}

void Gosu::TextBuilder::flush_current_line(EndOfLineReason reason)
{
    if (m_current_line.empty()) {
        if (reason == END_OF_PARAGRAPH) allocate_next_line();
        return;
    }

    allocate_next_line();

    // Remove trailing whitespace so that justifying the text across the line works.
    if (m_current_line.back().is_whitespace) m_current_line.pop_back();

    // Shouldn't happen because the first word on a line should never be whitespace.
    assert(!m_current_line.empty());

    double words_width = 0, whitespace_width = 0;
    for (const auto& word : m_current_line) {
        (word.is_whitespace ? whitespace_width : words_width) += word.width;
    }

    double x = 0;
    if (m_align == AL_RIGHT) {
        x = m_result.width() - words_width - whitespace_width;
    }
    else if (m_align == AL_CENTER) {
        x = (m_result.width() - words_width - whitespace_width) / 2.0;
    }

    double whitespace_factor = 1.0;
    if (m_align == AL_JUSTIFY && whitespace_width != 0 && reason == LINE_TOO_LONG) {
        whitespace_factor = (m_result.width() - words_width) / whitespace_width;
    }

    double y = (m_used_lines - 1) * (m_font_height + m_line_spacing);

    for (const auto& word : m_current_line) {
        if (word.is_whitespace) {
            x += word.width * whitespace_factor;
        }
        else {
            for (const auto& part : word.parts) {
                draw_text(m_result, x, y, part.color, part.text, m_font_name, m_font_height,
                          part.flags);
            }
            x += word.width;
        }
    }

    m_current_line.clear();
    m_current_line_width = 0;
}

void Gosu::TextBuilder::allocate_next_line()
{
    if (m_used_lines == m_allocated_lines) {
        m_allocated_lines += 10;
        resize_to_allocated_lines();
    }

    ++m_used_lines;
}

void Gosu::TextBuilder::resize_to_allocated_lines()
{
    double new_height =
            m_font_height * m_allocated_lines + m_line_spacing * std::max(0, m_allocated_lines - 1);
    m_result.resize(m_result.width(), ceil(new_height));
}

Gosu::TextBuilder::TextBuilder(const std::string& font_name, int font_height, int line_spacing,
                               int width, Alignment align)
: m_font_name{font_name},
  m_font_height{font_height * 1.0},
  m_line_spacing{line_spacing * 1.0},
  m_align{align}
{
    // This class uses result.width() to remember its destination width, so store it in there.
    m_result.resize(width, 0);
}

Gosu::Bitmap Gosu::TextBuilder::move_into_bitmap() &&
{
    flush_current_line(END_OF_TEXT);

    // Shrink to fit the currently used height.
    m_allocated_lines = m_used_lines;
    resize_to_allocated_lines();
    return std::move(m_result);
}

void Gosu::TextBuilder::feed_word(std::vector<FormattedString>&& word)
{
    WordInfo new_word(m_font_name, m_font_height, word);

    if (m_current_line_width + new_word.width > m_result.width()) {
        // Can't fit it on the same line as before, so flush the last line before adding the word to
        // the next line.
        flush_current_line(LINE_TOO_LONG);

        if (new_word.is_whitespace) {
            // Do not wrap trailing whitespace onto the start of the next line - discard this word.
            return;
        }
    }

    m_current_line.emplace_back(new_word);
    m_current_line_width += new_word.width * 1.0;

    if (new_word.is_end_of_line) {
        flush_current_line(END_OF_PARAGRAPH);
    }
}
