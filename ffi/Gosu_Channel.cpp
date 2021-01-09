#include "Gosu_Channel.h"
#include <Gosu/Audio.hpp>

struct Gosu_Channel
{
    Gosu::Channel channel;
};

GOSU_FFI_API void Gosu_Channel_destroy(Gosu_Channel* channel)
{
    delete channel;
}

GOSU_FFI_API bool Gosu_Channel_playing(Gosu_Channel* channel)
{
    return channel->channel.playing();
}

GOSU_FFI_API void Gosu_Channel_pause(Gosu_Channel* channel)
{
    channel->channel.pause();
}

GOSU_FFI_API bool Gosu_Channel_paused(Gosu_Channel* channel)
{
    return channel->channel.paused();
}

GOSU_FFI_API void Gosu_Channel_resume(Gosu_Channel* channel)
{
    channel->channel.resume();
}

GOSU_FFI_API void Gosu_Channel_stop(Gosu_Channel* channel)
{
    channel->channel.stop();
}

GOSU_FFI_API void Gosu_Channel_set_volume(Gosu_Channel* channel, double volume)
{
    channel->channel.set_volume(volume);
}

GOSU_FFI_API void Gosu_Channel_set_speed(Gosu_Channel* channel, double speed)
{
    channel->channel.set_speed(speed);
}

GOSU_FFI_API void Gosu_Channel_set_pan(Gosu_Channel* channel, double pan)
{
    channel->channel.set_pan(pan);
}
