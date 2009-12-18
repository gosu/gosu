#include <Gosu/Bitmap.hpp>
#include <vector>

void Gosu::applyColorKey(Bitmap& bitmap, Color key)
{
    std::vector<Color> surroundingColors;
    surroundingColors.reserve(4);

    for (unsigned y = 0; y < bitmap.height(); ++y)
        for (unsigned x = 0; x < bitmap.width(); ++x)
            if (bitmap.getPixel(x, y) == key)
            {
                surroundingColors.clear();
                if (x > 0 && bitmap.getPixel(x - 1, y) != key)
                    surroundingColors.push_back(bitmap.getPixel(x - 1, y));
                if (x < bitmap.width() - 1 && bitmap.getPixel(x + 1, y) != key)
                    surroundingColors.push_back(bitmap.getPixel(x + 1, y));
                if (y > 0 && bitmap.getPixel(x, y - 1) != key)
                    surroundingColors.push_back(bitmap.getPixel(x, y - 1));
                if (y < bitmap.height() - 1 && bitmap.getPixel(x, y + 1) != key)
                    surroundingColors.push_back(bitmap.getPixel(x, y + 1));

                if (surroundingColors.empty())
                {
                    bitmap.setPixel(x, y, Color::NONE);
                    continue;
                }

                unsigned red = 0, green = 0, blue = 0;
                for (unsigned i = 0; i < surroundingColors.size(); ++i)
                {
                    red += surroundingColors[i].red();
                    green += surroundingColors[i].green();
                    blue += surroundingColors[i].blue();
                }
                bitmap.setPixel(x, y, Color(0, red / surroundingColors.size(),
                    green / surroundingColors.size(), blue / surroundingColors.size()));
            }
}
