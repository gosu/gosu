#include "AudioImpl.hpp"
#include <stdexcept>
#include <string>
using namespace std;

// Per-platform limit for playing OpenAL sources: https://stackoverflow.com/a/3203888
#ifdef GOSU_IS_IPHONE
static const unsigned CHANNELS = 32;
#else
static const unsigned CHANNELS = 255;
#endif

static ALCdevice* _device;
static ALCcontext* _context;
static ALuint _sources[CHANNELS];
static int _tokens[CHANNELS];

void Gosu::al_initialize()
{
    if (_device) return;

    _device = alcOpenDevice(nullptr);
    _context = alcCreateContext(_device, nullptr);
    alcMakeContextCurrent(_context);
    alGenSources(CHANNELS, _sources);

    // Don't worry about ever closing the OpenAL device; it's hard to know when to clean up within
    // the scope of a Ruby C extension. Let the operating system clean up after us.
}

bool Gosu::al_initialized()
{
    return _device != nullptr;
}

ALCdevice* Gosu::al_device()
{
    al_initialize();
    
    return _device;
}

ALCcontext* Gosu::al_context()
{
    al_initialize();
    
    return _context;
}

Gosu::Channel Gosu::allocate_channel()
{
    al_initialize();

    // Start looking at index 1 to keep one free channel for songs.
    for (int i = 1; i < CHANNELS; ++i) {
        // Do not interrupt any playing or paused samples.
        ALint state;
        alGetSourcei(_sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING || state == AL_PAUSED) continue;
        
        // Increase the token to invalidate the last playing Channel with this index.
        return Channel(i, ++_tokens[i]);
    }

    // No free channel, return an object that is immediately expired.
    return Channel();
}

bool Gosu::channel_expired(int channel, int token)
{
    return channel < 0 || channel >= CHANNELS || _tokens[channel] != token;
}

ALuint Gosu::al_source_for_channel(int channel)
{
    if (channel < 0 || channel >= CHANNELS) {
        throw invalid_argument("No such channel: " + to_string(channel));
    }
    return _sources[channel];
}

ALuint Gosu::al_source_for_songs()
{
    return _sources[0];
}
