#pragma once

#include <Gosu/Audio.hpp>
#include <Gosu/Platform.hpp>
#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace Gosu
{
    void al_initialize();
    bool al_initialized();
    void al_shutdown();
    
    // Will initialize OpenAL if necessary.
    ALCdevice* al_device();
    // Will initialize OpenAL if necessary.
    ALCcontext* al_context();
    
    const int NO_CHANNEL = -1;
    
    // Returns an expired Channel if no channel is free.
    // Will initialize OpenAL if necessary.
    Channel allocate_channel();
    
    bool channel_expired(int channel, int token);
    ALuint al_source_for_channel(int channel);
    ALuint al_source_for_songs();
}
