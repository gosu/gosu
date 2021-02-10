#include <Gosu/Platform.hpp>
#ifdef GOSU_IS_ANDROID
#include <SDL.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>

using namespace std;

struct Gosu::File::Impl
{
    AAsset* asset;
    
    ~Impl()
    {
        if (asset) {
            __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "Closing asset...\n");
            AAsset_close(asset);
        }
    }
};

AAssetManager *m_asset_manager;

Gosu::File::File(const string& filename, FileMode mode)
: pimpl(new Impl)
{
    if (m_asset_manager == nullptr) {
        auto* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

        auto activity = (jobject)SDL_AndroidGetActivity();

        jclass activity_class = env->GetObjectClass(activity);

        jmethodID activity_class_getAssets = env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
        jobject asset_manager = env->CallObjectMethod(activity, activity_class_getAssets); // activity.getAssets();
        jobject global_asset_manager = env->NewGlobalRef(asset_manager);

        m_asset_manager = AAssetManager_fromJava(env, global_asset_manager);

        AAssetDir *asset_dir = AAssetManager_openDir(m_asset_manager, "");
        // List and print files for debugging
        while (const char* asset_name = AAssetDir_getNextFileName(asset_dir)) {
            __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "Asset filename: %s\n", asset_name);
        }

        AAssetDir_close(asset_dir);
    }

    __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "Opening asset: %s\n", filename.c_str());
    pimpl->asset = AAssetManager_open(m_asset_manager, filename.c_str(), mode);
}

Gosu::File::~File()
{
}

size_t Gosu::File::size() const
{
    // TODO: Error checking?
    return AAsset_getLength(pimpl->asset);
}

void Gosu::File::resize(size_t new_size)
{
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "File resizing [no-op]\n");
}

void Gosu::File::read(size_t offset, size_t length, void* dest_buffer) const
{
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "Reading asset with length: %d/%d...\n", length, size());

    AAsset_seek(pimpl->asset, offset, SEEK_SET);
    AAsset_read(pimpl->asset, dest_buffer, length);

    __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "Read asset.\n");
}

void Gosu::File::write(size_t offset, size_t length, const void* source_buffer)
{
    // TODO
    __android_log_print(android_LogPriority::ANDROID_LOG_INFO, "Gosu", "Writing asset [no-op]\n");
}

#endif
