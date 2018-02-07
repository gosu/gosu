#include "FormattedString.hpp"
#include "GraphicsImpl.hpp"
#include <Gosu/Utility.hpp>

#include <cstring>
#include <cstdlib>

using namespace std;


class Gosu::MarkupParser
{
    // The destination vector that substrings will be appended to.
    vector<FormattedSubstring>& substrings;

    // The current parser position.
    const char* markup;
    // The start of the current substring range.
    const char* substring;
    
    // Current b/i/u counters (they can stack).
    int b, i, u;
    // Current color stack.
    vector<Color> c{ Color::WHITE };
    
    unsigned flags() const
    {
        unsigned flags = 0;
        if (b > 0) flags |= Gosu::FF_BOLD;
        if (i > 0) flags |= Gosu::FF_ITALIC;
        if (u > 0) flags |= Gosu::FF_UNDERLINE;
        return flags;
    }
    
    template<size_t N>
    bool match_flush_and_skip(const char (&chars)[N])
    {
        if (strncmp(markup, chars, N) == 0) {
            flush_current_substring();
            markup += N;
            substring = markup;
            return true;
        }
        return false;
    }
    
    bool parse_markup()
    {
        if (match_flush_and_skip("<b>")) {
            b += 1;
            return true;
        }
        if (match_flush_and_skip("</b>")) {
            b -= 1;
            return true;
        }
        if (match_flush_and_skip("<u>")) {
            u += 1;
            return true;
        }
        if (match_flush_and_skip("</u>")) {
            u -= 1;
            return true;
        }
        if (match_flush_and_skip("<i>")) {
            i += 1;
            return true;
        }
        if (match_flush_and_skip("</i>")) {
            i -= 1;
            return true;
        }
        if (match_flush_and_skip("</c>")) {
            if (c.size() > 1) {
                c.pop_back();
            }
            else {
                // If the user pops the color stack empty, go back to the default color.
                c[0] = Color::WHITE;
            }
            return true;
        }

        if (strncmp(markup, "<c=", 3) == 0) {
            // Count hex chars following the <c= string.
            const char* hex = markup + 3;
            const char* valid_hex_chars = "0123456789ABCDEFabcdef";
            size_t hex_chars = strspn(hex, valid_hex_chars);
            
            if (*(hex + hex_chars) != '>' || (hex_chars != 3 && hex_chars != 6 && hex_chars != 8)) {
                // Does not match <c=[0-9A-Fa-f]{3,6,8}> -> not considered valid markup.
                return false;
            }

            flush_current_substring();

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
            
            c.push_back(argb);
            
            markup += (4 + hex_chars);
            substring = markup;
            return true;
        }
        
        // Everything else is not considered markup.
        return false;
    }
    
    bool parse_entity()
    {
        if (match_flush_and_skip("&lt")) {
            flush_substring("<");
            return true;
        }
        if (match_flush_and_skip("&gt")) {
            flush_substring(">");
            return true;
        }
        if (match_flush_and_skip("&amp;")) {
            flush_substring("&");
            return true;
        }

        if (*markup == '&') {
            const char* entity_name = markup + 1;
            const char* end_of_entity_markers = "\t\n ;";
            size_t entity_name_length = strcspn(entity_name, end_of_entity_markers);
            
            if (entity_name_length == 0 || *(entity_name + entity_name_length) != ';') {
                // Does not match &[^\t\n ;]+; -> not considered a valid entity.
                return false;
            }
            
            FormattedSubstring fs;
            fs.entity.assign(entity_name, entity_name + entity_name_length);
            fs.flags = flags();
            fs.color = c.back();
            substrings.push_back(fs);
            
            markup += (2 + entity_name_length);
            substring = markup;
            return true;
        }

        // Everything else is not considered an entity.
        return false;
    }

    void flush_current_substring()
    {
        if (substring != markup) {
            flush_substring(string(substring, markup));
        }
    }
    
    void flush_substring(string substring)
    {
        FormattedSubstring fs;
        fs.string = substring;
        fs.flags = flags();
        fs.color = c.back();
        
        if (! substrings.empty() && substrings.back().can_be_merged_with(fs)) {
            substrings.back().string.append(fs.string);
        }
        else {
            substrings.push_back(fs);
        }
    }

public:
    MarkupParser(vector<FormattedSubstring>& substrings, const char* markup, unsigned base_flags)
    : substrings(substrings), markup(markup)
    {
        int b = (base_flags & FF_BOLD)      ? 1 : 0;
        int i = (base_flags & FF_ITALIC)    ? 1 : 0;
        int u = (base_flags & FF_UNDERLINE) ? 1 : 0;
    }

    void parse()
    {
        substring = markup;
        
        while (*markup) {
            if (*markup == '<' && parse_markup()) {
                continue;
            }
            if (*markup == '&' && parse_entity()) {
                continue;
            }
            // This character was harmless, let's look at the next one, building up our substring.
            ++markup;
        }
        
        flush_current_substring();
    }
};

Gosu::FormattedString::FormattedString()
{
}

Gosu::FormattedString::FormattedString(const char* markup, unsigned base_flags)
{
    MarkupParser parser(substrings, markup, base_flags);
    parser.parse();
}

