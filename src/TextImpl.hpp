#pragma once

#include <Gosu/Bitmap.hpp>
#include <string>

namespace Gosu
{
    int text_width_ttf(const std::string& text,
                       const std::string& filename, int font_height, unsigned font_flags);

    void draw_text_ttf(Bitmap& bitmap, const std::string& text, int x, int y, Color c,
                       const std::string& filename, int font_height, unsigned font_flags);
}
