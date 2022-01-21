#include <Gosu/Text.hpp>
#include "GraphicsImpl.hpp"
#include "MarkupParser.hpp"
#include "TextBuilder.hpp"
#include "TrueTypeFont.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

double Gosu::text_width(const std::u32string& text, //
                        const std::string& font_name, double font_height, unsigned font_flags)
{
    if (font_height <= 0) throw std::invalid_argument{"font_height must be > 0"};
    if (font_flags >= FF_COMBINATIONS) throw std::invalid_argument{"Invalid font_flags"};

    TrueTypeFont& font = font_by_name(font_name, font_flags);
    return font.draw_text(text, font_height, nullptr, 0, 0, Gosu::Color::NONE);
}

double Gosu::draw_text(Bitmap& bitmap, double x, double y, Color c, const std::u32string& text,
                       const std::string& font_name, double font_height, unsigned font_flags)
{
    if (font_height <= 0) throw std::invalid_argument("font_height must be > 0");
    if (font_flags >= FF_COMBINATIONS) throw std::invalid_argument("Invalid font_flags");

    TrueTypeFont& font = font_by_name(font_name, font_flags);
    return font.draw_text(text, font_height, &bitmap, x, y, c);
}

Gosu::Bitmap Gosu::layout_text(const std::string& text, const std::string& font_name, //
                               double font_height, double line_spacing, int width,    //
                               Alignment align, unsigned font_flags)
{
    // Escape all markup and delegate to layout_markup.
    return layout_markup(escape_markup(text), font_name, font_height, line_spacing, width, align,
                         font_flags);
}

Gosu::Bitmap Gosu::layout_markup(const std::string& markup, const std::string& font_name, //
                                 double font_height, double line_spacing, int width,      //
                                 Alignment align, unsigned font_flags)
{
    if (font_height <= 0) throw std::invalid_argument{"font_height must be > 0"};
    if (line_spacing < -font_height) throw std::invalid_argument{"line_spacing < -font_height"};
    if (font_flags >= FF_COMBINATIONS) throw std::invalid_argument{"Invalid font_flags"};

    if (width >= 0) {
        TextBuilder text_builder{font_name, static_cast<int>(font_height),
                                 static_cast<int>(line_spacing), width, align};

        // Feed all formatted substrings to the TextBuilder, which will construct the result.
        // Split the input string into words, because this method implements word-wrapping.
        MarkupParser parser{font_flags, true, [&text_builder](std::vector<FormattedString> word) {
                                text_builder.feed_word(move(word));
                            }};
        parser.parse(markup);

        return std::move(text_builder).move_into_bitmap();
    }
    else {
        std::vector<std::vector<FormattedString>> lines;

        // Split the text into lines (split_words = false) since this method does not wrap lines.
        MarkupParser parser{font_flags, false, [&lines](std::vector<FormattedString>&& line) {
                                // Remove trailing \n characters from each line to avoid errors from
                                // Gosu::text_width().
                                if (!line.back().text.empty() && line.back().text.back() == '\n') {
                                    line.back().text.pop_back();
                                }

                                lines.emplace_back(line);
                            }};
        parser.parse(markup);

        if (lines.empty()) return Bitmap();

        // Measure every part of every line.
        std::vector<double> line_widths;
        double max_width = 0;
        for (auto& line : lines) {
            line_widths.push_back(0);
            for (auto& part : line) {
                line_widths.back() += text_width(part.text, font_name, font_height, part.flags);
            }
            max_width = std::max(max_width, line_widths.back());
        }

        double height = lines.size() * font_height + (lines.size() - 1.0) * line_spacing;
        Bitmap result{static_cast<int>(std::ceil(max_width)), static_cast<int>(std::ceil(height))};

        // Render every part of every line.
        double y = 0;
        for (int i = 0; i < lines.size(); ++i) {
            double x = 0;
            if (align == AL_CENTER) {
                x = (result.width() - line_widths[i]) / 2;
            }
            else if (align == AL_RIGHT) {
                x = result.width() - line_widths[i];
            }

            for (auto& part : lines[i]) {
                x = draw_text(result, x, y, part.color, part.text, font_name, font_height,
                              part.flags);
            }
            y += (font_height + line_spacing);
        }

        return result;
    }
}
