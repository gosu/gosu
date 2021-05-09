#include "Gosu_FFI_internal.h"

#ifdef ENABLE_FFMPEG

GOSU_FFI_API Gosu_Video* Gosu_Video_create(const char* filename, unsigned image_flags)
{
    return Gosu_translate_exceptions([=] { return new Gosu_Video{Gosu::Video(filename, image_flags)}; });
}

GOSU_FFI_API Gosu_Video* Gosu_Video_create_scaled(const char* filename, int width, int height, unsigned image_flags)
{
    return Gosu_translate_exceptions([=] { return new Gosu_Video{Gosu::Video(filename, width, height, image_flags)}; });
}

#endif