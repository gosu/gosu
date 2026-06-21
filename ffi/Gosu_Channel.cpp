#include "Gosu_FFI_internal.h"

GOSU_FFI_API void Gosu_Channel_destroy(Gosu_Channel* channel)
{
    delete channel;
}

GOSU_FFI_API bool Gosu_Channel_playing(Gosu_Channel* channel)
{
    return Gosu_translate_exceptions([=] {
        return channel->channel.playing();
    });
}

GOSU_FFI_API void Gosu_Channel_pause(Gosu_Channel* channel)
{
    Gosu_translate_exceptions([=] {
        channel->channel.pause();
    });
}

GOSU_FFI_API bool Gosu_Channel_paused(Gosu_Channel* channel)
{
    return Gosu_translate_exceptions([=] {
        return channel->channel.paused();
    });
}

GOSU_FFI_API void Gosu_Channel_resume(Gosu_Channel* channel)
{
    Gosu_translate_exceptions([=] {
        channel->channel.resume();
    });
}

GOSU_FFI_API void Gosu_Channel_stop(Gosu_Channel* channel)
{
    Gosu_translate_exceptions([=] {
        channel->channel.stop();
    });
}

GOSU_FFI_API void Gosu_Channel_set_volume(Gosu_Channel* channel, double volume)
{
    Gosu_translate_exceptions([=] {
        channel->channel.set_volume(volume);
    });
}

GOSU_FFI_API void Gosu_Channel_set_speed(Gosu_Channel* channel, double speed)
{
    Gosu_translate_exceptions([=] {
        channel->channel.set_speed(speed);
    });
}

GOSU_FFI_API void Gosu_Channel_set_pan(Gosu_Channel* channel, double pan)
{
    Gosu_translate_exceptions([=] {
        channel->channel.set_pan(pan);
    });
}
