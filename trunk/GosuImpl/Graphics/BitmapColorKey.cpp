#include <Gosu/Bitmap.hpp>
#include <vector>

// IMPR: Algo is very simple ATM, still not very performant.

void Gosu::applyColorKey(Bitmap& bitmap, Color key)
{
    std::vector<Color> surColors;
    surColors.reserve(4);

    for (unsigned y = 0; y < bitmap.height(); ++y)
        for (unsigned x = 0; x < bitmap.width(); ++x)
            if (bitmap.getPixel(x, y) == key)
            {
                surColors.clear();
                if (x > 0 && bitmap.getPixel(x - 1, y) != key)
                    surColors.push_back(bitmap.getPixel(x - 1, y));
                if (x < bitmap.width() - 1 && bitmap.getPixel(x + 1, y) != key)
                    surColors.push_back(bitmap.getPixel(x + 1, y));
                if (y > 0 && bitmap.getPixel(x, y - 1) != key)
                    surColors.push_back(bitmap.getPixel(x, y - 1));
                if (y < bitmap.height() - 1 && bitmap.getPixel(x, y + 1) != key)
                    surColors.push_back(bitmap.getPixel(x, y + 1));

                if (surColors.empty())
                {
                    bitmap.setPixel(x, y, Colors::none);
                    continue;
                }

                unsigned red = 0, green = 0, blue = 0;
                for (unsigned i = 0; i < surColors.size(); ++i)
                {
                    red += surColors[i].red();
                    green += surColors[i].green();
                    blue += surColors[i].blue();
                }
                bitmap.setPixel(x, y, Color(0, red / surColors.size(),
                    green / surColors.size(), blue / surColors.size()));
            }
}
