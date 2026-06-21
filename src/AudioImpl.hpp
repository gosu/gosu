#pragma once

#include <Gosu/Audio.hpp>
#include <Gosu/Platform.hpp>
#ifdef GOSU_IS_IPHONE
// Ignore OpenAL deprecation warnings. If iOS stops shipping OpenAL, it's more likely that we bundle our own version
// of it than that we switch to another audio API.
#define OPENAL_DEPRECATED
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

namespace Gosu
{
    void al_initialize();
    bool al_initialized();

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
