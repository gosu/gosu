#include <Gosu/Audio.hpp>
#include "Gosu_Channel.h"

extern "C" {

Gosu_Channel *Gosu_Channel_create(Gosu_Channel *channel)
{
    return channel;
}

bool Gosu_Channel_playing(Gosu_Channel *channel)
{
    return reinterpret_cast<Gosu::Channel *>(channel)->playing();
}
void Gosu_Channel_pause(Gosu_Channel *channel)
{
    reinterpret_cast<Gosu::Channel *>(channel)->pause();
}
bool Gosu_Channel_paused(Gosu_Channel *channel)
{
    return reinterpret_cast<Gosu::Channel *>(channel)->paused();
}
void Gosu_Channel_resume(Gosu_Channel *channel)
{
    reinterpret_cast<Gosu::Channel *>(channel)->resume();
}
void Gosu_Channel_stop(Gosu_Channel *channel)
{
    reinterpret_cast<Gosu::Channel *>(channel)->stop();
}

void Gosu_Channel_set_volume(Gosu_Channel *channel, double volume)
{
    reinterpret_cast<Gosu::Channel *>(channel)->set_volume(volume);
}
void Gosu_Channel_set_speed(Gosu_Channel *channel, double speed)
{
    reinterpret_cast<Gosu::Channel *>(channel)->set_speed(speed);
}
void Gosu_Channel_set_pan(Gosu_Channel *channel, double pan)
{
    reinterpret_cast<Gosu::Channel *>(channel)->set_pan(pan);
}

void Gosu_Channel_destroy(Gosu_Channel *channel)
{
    delete (reinterpret_cast<Gosu::Channel *>(channel));
}

}