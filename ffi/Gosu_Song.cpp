#include "Gosu_Song.h"
#include <Gosu/Audio.hpp>

struct Gosu_Song : Gosu::Song
{
    [[maybe_unused]] // silence buggy warning - this is being used, see below.
    explicit Gosu_Song(const std::string& filename)
    : Gosu::Song{filename}
    {
    }
};

GOSU_FFI_API Gosu_Song* Gosu_Song_create(const char* filename)
{
    return new Gosu_Song{filename};
}

GOSU_FFI_API void Gosu_Song_destroy(Gosu_Song* song)
{
    delete song;
}

GOSU_FFI_API void Gosu_Song_play(Gosu_Song* song, bool looping)
{
    song->play(looping);
}

GOSU_FFI_API bool Gosu_Song_playing(Gosu_Song* song)
{
    return song->playing();
}

GOSU_FFI_API void Gosu_Song_pause(Gosu_Song* song)
{
    song->pause();
}

GOSU_FFI_API bool Gosu_Song_paused(Gosu_Song* song)
{
    return song->paused();
}

GOSU_FFI_API void Gosu_Song_stop(Gosu_Song* song)
{
    song->stop();
}

GOSU_FFI_API double Gosu_Song_volume(Gosu_Song* song)
{
    return song->volume();
}

GOSU_FFI_API void Gosu_Song_set_volume(Gosu_Song* song, double volume)
{
    return song->set_volume(volume);
}

GOSU_FFI_API Gosu_Song* Gosu_Song_current_song(void)
{
    return reinterpret_cast<Gosu_Song*>(Gosu::Song::current_song());
}
