#include "../../dependencies/stb_image.c"

#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>
#include <cstring>

void Gosu::loadImageFile(Gosu::Bitmap& bitmap, const std::wstring& filename)
{
    int x, y, n;
    unsigned char *data = stbi_load(Gosu::narrow(filename).c_str(), &x, &y, &n, 4);
    bitmap.resize(x, y);
    std::memcpy(bitmap.data(), data, x * y * 4);
    stbi_image_free(data);
}

void Gosu::loadImageFile(Bitmap& bitmap, Reader input)
{
    // TODO use this
    // extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
    // or the one with callbacks...
    throw "NYI";
}

void Gosu::saveImageFile(const Bitmap& bitmap, const std::wstring& filename)
{
    throw "NYI";
}

void Gosu::saveImageFile(const Bitmap& bitmap, Gosu::Writer writer, const std::wstring& formatHint)
{
    throw "NYI";
}
