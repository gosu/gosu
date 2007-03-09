#ifndef RUBYGOSU_SONG_HPP
#define RUBYGOSU_SONG_HPP

#include <RubyGosu/RubyGosu.hpp>

namespace RubyGosu
{
    void registerSong(VALUE module);
    VALUE Song_initialize(VALUE self, VALUE windowV, VALUE filenameV,
        VALUE typeV);
    VALUE Song_play(VALUE self);
    VALUE Song_stop(VALUE self);
    VALUE Song_playing(VALUE self);
}

#endif
