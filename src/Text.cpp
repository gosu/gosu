#include "MarkupParser.hpp"
#include "TextBuilder.hpp"
#include "GraphicsImpl.hpp"
#include <Gosu/Text.hpp>
#include <cassert>
#include <algorithm>
#include <vector>
using namespace std;

Gosu::Bitmap Gosu::create_text(const string& text, const string& font_name, int font_height,
                               int line_spacing, int width, Alignment align, unsigned font_flags)
{
    if (font_height <= 0)            throw invalid_argument("font_height must be > 0");
    if (width <= 0)                  throw invalid_argument("width must be > 0");
    if (line_spacing < -font_height) throw invalid_argument("line_spacing must be ≥ -font_height");

    TextBuilder text_builder(font_name, font_height, line_spacing, width, align);
    
    // Feed all formatted substrings to the TextBuilder, which will construct the resulting bitmap.
    // Split the input string into words here, because this method implements word-wrapping.
    MarkupParser(text.c_str(), font_flags, true, [&text_builder](vector<FormattedString> word) {
        text_builder.feed_word(move(word));
    }).parse();
    
    return text_builder.move_into_bitmap();
}

Gosu::Bitmap Gosu::create_text(const string& text, const string& font_name, int font_height,
                               unsigned font_flags)
{
    if (font_height <= 0) throw invalid_argument("font_height must be > 0");

    vector<vector<FormattedString>> lines;

    // Split the text into lines (split_words = false) since this method does not break lines.
    MarkupParser(text.c_str(), font_flags, false, [&lines](vector<FormattedString>&& line) {
        // Remove trailing \n characters from each line to avoid errors from Gosu::text_width().
        if (line.back().string.back() == '\n') {
            line.back().string.pop_back();
        }
            
        lines.emplace_back(line);
    }).parse();
    
    // Measure every part of every line.
    int width = 0;
    for (auto& line : lines) {
        int line_width = 0;
        for (auto& part : line) {
            line_width += text_width(part.string, font_name, font_height, part.flags);
        }
        width = max(width, line_width);
    }
    
    Bitmap result(width, static_cast<int>(lines.size() * font_height));
    
    // Render every part of every line.
    int y = 0;
    for (auto& line : lines) {
        int x = 0;
        for (auto& part : line) {
            x += draw_text(result, part.string, x, y, Color::WHITE, font_name, font_height);
        }
        y += font_height;
    }
    
    return result;
}
