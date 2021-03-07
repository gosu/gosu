#include "Gosu_FFI_internal.h"

GOSU_FFI_API Gosu_Song* Gosu_Song_create(const char* filename)
{
    return Gosu_translate_exceptions([=] {
        return new Gosu_Song{filename};
    });
}

GOSU_FFI_API void Gosu_Song_destroy(Gosu_Song* song)
{
    Gosu_translate_exceptions([=] {
        delete song;
    });
}

GOSU_FFI_API void Gosu_Song_play(Gosu_Song* song, bool looping)
{
    Gosu_translate_exceptions([=] {
        song->play(looping);
    });
}

GOSU_FFI_API bool Gosu_Song_playing(Gosu_Song* song)
{
    return Gosu_translate_exceptions([=] {
        return song->playing();
    });
}

GOSU_FFI_API void Gosu_Song_pause(Gosu_Song* song)
{
    Gosu_translate_exceptions([=] {
        song->pause();
    });
}

GOSU_FFI_API bool Gosu_Song_paused(Gosu_Song* song)
{
    return Gosu_translate_exceptions([=] {
        return song->paused();
    });
}

GOSU_FFI_API void Gosu_Song_stop(Gosu_Song* song)
{
    Gosu_translate_exceptions([=] {
        song->stop();
    });
}

GOSU_FFI_API double Gosu_Song_volume(Gosu_Song* song)
{
    return Gosu_translate_exceptions([=] {
        return song->volume();
    });
}

GOSU_FFI_API void Gosu_Song_set_volume(Gosu_Song* song, double volume)
{
    Gosu_translate_exceptions([=] {
        return song->set_volume(volume);
    });
}

GOSU_FFI_API Gosu_Song* Gosu_Song_current_song(void)
{
    return Gosu_translate_exceptions([=] {
        return reinterpret_cast<Gosu_Song*>(Gosu::Song::current_song());
    });
}
