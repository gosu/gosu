#include <Gosu/Audio.hpp>
#include "AudioImpl.hpp"
#include <algorithm>

/// Returns the current state of a source.
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
: m_channel{NO_CHANNEL},
  m_token{0}
{
}

Gosu::Channel::Channel(int channel, int token)
: m_channel{channel},
  m_token{token}
{
}

int Gosu::Channel::current_channel() const
{
    if (m_channel != NO_CHANNEL && channel_expired(m_channel, m_token)) {
        m_channel = NO_CHANNEL;
    }
    return m_channel;
}

bool Gosu::Channel::playing() const
{
    if (current_channel() == NO_CHANNEL) return false;

    return state(m_channel) == AL_PLAYING;
}

bool Gosu::Channel::paused() const
{
    if (current_channel() == NO_CHANNEL) return false;

    return state(m_channel) == AL_PAUSED;
}

void Gosu::Channel::pause()
{
    if (playing()) {
        ALuint source = al_source_for_channel(m_channel);
        alSourcePause(source);
    }
}

void Gosu::Channel::resume()
{
    if (paused()) {
        ALuint source = al_source_for_channel(m_channel);
        alSourcePlay(source);
    }
}

void Gosu::Channel::stop()
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(m_channel);
    alSourceStop(source);
    m_channel = NO_CHANNEL;
}

void Gosu::Channel::set_volume(double volume)
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(m_channel);
    alSourcef(source, AL_GAIN, static_cast<ALfloat>(std::max(volume, 0.0)));
}

void Gosu::Channel::set_pan(double pan)
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(m_channel);
    alSource3f(source, AL_POSITION, static_cast<ALfloat>(pan * 10), 0, 0);
}

void Gosu::Channel::set_speed(double speed)
{
    if (current_channel() == NO_CHANNEL) return;

    ALuint source = al_source_for_channel(m_channel);
    alSourcef(source, AL_PITCH, static_cast<ALfloat>(speed));
}
