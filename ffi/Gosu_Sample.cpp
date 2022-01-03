#include "Gosu_FFI_internal.h"

GOSU_FFI_API Gosu_Sound* Gosu_Sound_create(const char* filename)
{
    return Gosu_translate_exceptions([=] { //
        return new Gosu_Sound{Gosu::Sound{filename}};
    });
}

GOSU_FFI_API void Gosu_Sound_destroy(Gosu_Sound* sound)
{
    Gosu_translate_exceptions([=] { //
        delete sound;
    });
}

GOSU_FFI_API Gosu_Channel* Gosu_Sound_play(Gosu_Sound* sound, double volume, double speed,
                                           bool looping)
{
    return Gosu_translate_exceptions([=] { //
        return new Gosu_Channel{sound->sound.play(volume, speed, looping)};
    });
}

GOSU_FFI_API Gosu_Channel* Gosu_Sound_play_pan(Gosu_Sound* sound, double pan, double volume,
                                               double speed, bool looping)
{
    return Gosu_translate_exceptions([=] { //
        return new Gosu_Channel{sound->sound.play_pan(pan, volume, speed, looping)};
    });
}
