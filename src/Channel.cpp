#include <Gosu/Audio.hpp>
#include "AudioImpl.hpp"
using namespace std;

// Returns the current state of a source
static ALint state(int& channel)
{
    ALint state;
    alGetSourcei(Gosu::al_source_for_channel(channel), AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && state != AL_PAUSED) {
        channel = Gosu::NO_CHANNEL;
    }
    return state;
}

Gosu::Channel::Channel()
: channel(NO_CHANNEL), token(0)
{
}

Gosu::Channel::Channel(int channel, int token)
: channel(channel), token(token)
{
}

int Gosu::Channel::current_channel() const
{
    if (channel != NO_CHANNEL && channel_expired(channel, token)) {
        channel = NO_CHANNEL;
    }
    return channel;
}

bool Gosu::Channel::playing() const
{
    if (current_channel() == NO_CHANNEL) return false;
    
    return state(channel) == AL_PLAYING;
}

bool Gosu::Channel::paused() const
{
    if (current_channel() == NO_CHANNEL) return false;

    return state(channel) == AL_PAUSED;
}

void Gosu::Channel::pause()
{
    if (playing()) {
        ALuint source = al_source_for_channel(channel);
        alSourcePause(source);
    }
}

void Gosu::Channel::resume()
{
    if (paused()) {
        ALuint source = al_source_for_channel(channel);
        alSourcePlay(source);
    }
}

void Gosu::Channel::stop()
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(channel);
    alSourceStop(source);
    channel = NO_CHANNEL;
}

void Gosu::Channel::set_volume(double volume)
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(channel);
    alSourcef(source, AL_GAIN, max(volume, 0.0));
}

void Gosu::Channel::set_pan(double pan)
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(channel);
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
}

void Gosu::Channel::set_speed(double speed)
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(channel);
    alSourcef(source, AL_PITCH, speed);
}
