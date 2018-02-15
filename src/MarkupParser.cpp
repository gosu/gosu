#include "MarkupParser.hpp"
#include <Gosu/Utility.hpp>
#include <cstring>
using namespace std;


// Helper method for making Chinese/Japanese text break even in the absence of whitespace.
static bool should_allow_break_after_codepoint(int cp)
{
    return (cp >= 0x3040 && cp <= 0x3096) || // Hiragana
           (cp >= 0x30a0 && cp <= 0x30fa) || // Katakana
           (cp >= 0x4e00 && cp <= 0x9fff) || // CJK Unified Ideographs
           (cp >= 0x3400 && cp <= 0x4db5);   // CJK Unified Ideographs Extension A
}

unsigned Gosu::MarkupParser::flags() const
{
    unsigned flags = 0;
    if (b > 0) flags |= Gosu::FF_BOLD;
    if (i > 0) flags |= Gosu::FF_ITALIC;
    if (u > 0) flags |= Gosu::FF_UNDERLINE;
    return flags;
}

bool Gosu::MarkupParser::match_and_skip(const char* chars, size_t length)
{
    if (strncmp(markup, chars, length) == 0) {
        // Finish building the current substring (if any) before matching.
        add_current_substring();
        // Skip chars.
        markup += length;
        substring = markup;
        return true;
    }
    return false;
}

bool Gosu::MarkupParser::parse_markup()
{
    // Open and close bold text spans.
    if (match_and_skip("<b>")) {
        b += 1;
        return true;
    }
    if (match_and_skip("</b>")) {
        b -= 1;
        return true;
    }

    // Open and close underlined text spans.
    if (match_and_skip("<u>")) {
        u += 1;
        return true;
    }
    if (match_and_skip("</u>")) {
        u -= 1;
        return true;
    }

    // Open and close italic text spans.
    if (match_and_skip("<i>")) {
        i += 1;
        return true;
    }
    if (match_and_skip("</i>")) {
        i -= 1;
        return true;
    }

    // Reset to previous color.
    if (match_and_skip("</c>")) {
        if (c.size() > 1) {
            c.pop_back();
        }
        else {
            // If the user pops the color stack empty, go back to the default color.
            c[0] = Color::WHITE;
        }
        return true;
    }

    // Leave the trickiest case for last - changing the current text color.
    if (strncmp(markup, "<c=", 3) == 0) {
        // Count hex chars following the <c= string.
        const char* hex = markup + 3;
        const char* valid_hex_chars = "0123456789ABCDEFabcdef";
        size_t hex_chars = strspn(hex, valid_hex_chars);

        if (*(hex + hex_chars) != '>' || (hex_chars != 3 && hex_chars != 6 && hex_chars != 8)) {
            // Does not match <c=[0-9A-Fa-f]{3,6,8}> -> not considered valid markup.
            return false;
        }

        add_current_substring();

        auto argb = strtoul(hex, nullptr, 16);

        if (hex_chars == 3) {
            // Expand 0xrgb to 0xFFrrggbb:
            auto r = argb >> 8 & 0x7;
            auto g = argb >> 4 & 0x7;
            auto b = argb >> 0 & 0x7;
            argb = 0xff000000 | r << 20 | r << 16 | g << 12 | g << 8 | b << 4 | b << 0;
        }
        else if (hex_chars == 6) {
            // Expand 0xrrggbb to 0xFFrrggbb:
            argb = 0xff000000 | argb;
        }

        c.emplace_back(argb);

        markup += (4 + hex_chars);
        substring = markup;
        return true;
    }

    // Everything else is not considered markup.
    return false;
}

bool Gosu::MarkupParser::parse_escape_entity()
{
    // These are not entities (images) but escapes for markup characters.
    if (match_and_skip("&lt;")) {
        add_substring("<");
        return true;
    }
    if (match_and_skip("&gt;")) {
        add_substring(">");
        return true;
    }
    if (match_and_skip("&amp;")) {
        add_substring("&");
        return true;
    }

    // These are the only recognized entities - disregard anything else.
    return false;
}

void Gosu::MarkupParser::add_current_substring()
{
    if (substring != markup) {
        add_substring(string(substring, markup));
        substring = markup;
    }
}

void Gosu::MarkupParser::add_substring(string&& substring)
{
    FormattedString fstr;
    fstr.string = substring;
    fstr.flags = flags();
    fstr.color = c.back();

    if (! substrings.empty() && substrings.back().can_be_merged_with(fstr)) {
        substrings.back().string.append(move(fstr.string));
    }
    else {
        substrings.emplace_back(move(fstr));
    }
}

void Gosu::MarkupParser::flush_to_consumer()
{
    if (! substrings.empty()) {
        consumer(move(substrings));
        substrings.clear();
    }
}

Gosu::MarkupParser::MarkupParser(const char* markup, unsigned base_flags, bool split_words,
                                 function<void (vector<FormattedString>)> consumer)
: markup(markup), substring(markup), consumer(move(consumer))
{
    word_state = (split_words ? ADDING_WORD : IGNORE_WORDS);

    b = (base_flags & FF_BOLD)      ? 1 : 0;
    i = (base_flags & FF_ITALIC)    ? 1 : 0;
    u = (base_flags & FF_UNDERLINE) ? 1 : 0;
}

void Gosu::MarkupParser::parse()
{
    while (*markup) {
        if (*markup == '<' && parse_markup()) {
            continue;
        }
        if (*markup == '&' && parse_escape_entity()) {
            continue;
        }

        // The newline character always terminates the current line, regardless of whether this
        // parser is trying to split words.
        if (*markup == '\n') {
            // Explicitly add the trailing \n to the current substring so that the consumer can
            // distinguish between line breaks and word breaks in split_words mode.
            ++markup;
            add_current_substring();
            flush_to_consumer();
            // Avoid incrementing ++markup again.
            continue;
        }

        bool whitespace_except_newline = isspace(static_cast<int>(*markup));

        if (whitespace_except_newline && word_state == ADDING_WORD) {
            // We are in word-parsing mode, and this is was the end of a word.
            add_current_substring();
            flush_to_consumer();
            word_state = ADDING_WHITESPACE;
        } else if (!whitespace_except_newline && word_state == ADDING_WHITESPACE) {
            // We are in word-parsing mode, and this is was the start of a word.
            add_current_substring();
            flush_to_consumer();
            word_state = ADDING_WORD;
        }

        // TODO: Need to find a solution for breaking based on Unicode codepoints here.
        // (Asian languages without whitespace will not have linebreaks otherwise)
        // I guess that means using utf8proc? But does that even make sense if the TTF renderer
        // is based on UCS-4 codepoints? Maybe better to normalize to UCS-4 here and be done with
        // it? (i.e. make FormattedSubstring contain a vector<uint32_t> string)

        ++markup;
    }

    add_current_substring();
    flush_to_consumer();
}
