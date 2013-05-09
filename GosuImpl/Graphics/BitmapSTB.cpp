#include "../../dependencies/stb_image.c"

#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>
#include <cstring>
#include <android/native_activity.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>

#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "LD26", __VA_ARGS__))

namespace Gosu {
    extern android_app* app;
}

void Gosu::loadImageFile(Gosu::Bitmap& bitmap, const std::wstring& filename)
{
    AAssetManager* assetManager = app->activity->assetManager;
    // TODO mbstowcs is broken on Android - need to use iconv
    std::string narrowFilename(filename.begin(), filename.end());
    AAsset* asset = AAssetManager_open(assetManager, narrowFilename.c_str(), AASSET_MODE_UNKNOWN);
    assert(asset);
    Buffer buffer;
    buffer.resize(AAsset_getRemainingLength(asset));
    AAsset_read(asset, buffer.data(), buffer.size());

    int x, y, n;
    unsigned char *data = stbi_load_from_memory(static_cast<unsigned char*>(buffer.data()), buffer.size(), &x, &y, &n, 4);
    assert(data);
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
