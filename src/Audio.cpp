#include "AudioImpl.hpp"
#include "OggFile.hpp"
#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <cstdlib>
#include <algorithm>

#ifdef GOSU_IS_MAC
#import <Foundation/Foundation.h>
#include "AudioToolboxFile.hpp"
#else
#include "MPEGFile.hpp"
#include "SndFile.hpp"
#endif

#ifdef GOSU_IS_IPHONE
#import <AVFoundation/AVFoundation.h>
#endif

using namespace std;

static bool is_ogg_file(Gosu::Reader reader)
{
    if (reader.resource().size() < 4) return false;
    
    char bytes[4];
    reader.read(bytes, 4);
    return bytes[0] == 'O' && bytes[1] == 'g' && bytes[2] == 'g' && bytes[3] == 'S';
}

static bool is_ogg_file(const string& filename)
{
    Gosu::File file(filename);
    return is_ogg_file(file.front_reader());
}

static Gosu::Song* cur_song = nullptr;
static bool cur_song_looping;

struct Gosu::Sample::Impl
{
    ALuint buffer;
    double length;

    Impl(AudioFile&& audio_file)
    {
        al_initialize();
        alGenBuffers(1, &buffer);
        alBufferData(buffer, audio_file.format(), &audio_file.decoded_data().front(),
                     static_cast<ALsizei>(audio_file.decoded_data().size()),
                     audio_file.sample_rate());
        length = audio_file.length();
    }
    
    ~Impl()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffer isn't deleted after the context/device are shut down.
        if (!al_initialized()) return;

        alDeleteBuffers(1, &buffer);
    }
};

Gosu::Sample::Sample()
{
}

Gosu::Sample::Sample(const string& filename)
{
    if (is_ogg_file(filename)) {
        File file(filename);
        pimpl.reset(new Impl(OggFile(file.front_reader())));
        return;
    }
    
#ifdef GOSU_IS_MAC
    File file(filename);
    pimpl.reset(new Impl(AudioToolboxFile(file.front_reader())));
#else
    try {
        pimpl.reset(new Impl(SndFile(filename)));
    }
    catch (const runtime_error& ex) {
        File file(filename);
        pimpl.reset(new Impl(MPEGFile(file.front_reader())));
    }
#endif
}

Gosu::Sample::Sample(Gosu::Reader reader)
{
    if (is_ogg_file(reader)) {
        pimpl.reset(new Impl(OggFile(reader)));
        return;
    }

#ifdef GOSU_IS_MAC
    pimpl.reset(new Impl(AudioToolboxFile(reader)));
#else
    try {
        pimpl.reset(new Impl(SndFile(reader)));
    }
    catch (const runtime_error& ex) {
        pimpl.reset(new Impl(MPEGFile(reader)));
    }
#endif
}

double Gosu::Sample::length() const
{
    return pimpl ? pimpl->length : 0.0;
}

Gosu::Channel Gosu::Sample::play(double volume, double speed, bool looping) const
{
    return play_pan(0, volume, speed, looping);
}

Gosu::Channel Gosu::Sample::play_pan(double pan, double volume, double speed, bool looping) const
{
    if (!pimpl) return Channel();

    Channel channel = allocate_channel();
    
    // Couldn't allocate a free channel.
    if (channel.current_channel() == NO_CHANNEL) return channel;
    
    ALuint source = al_source_for_channel(channel.current_channel());
    alSourcei(source, AL_BUFFER, pimpl->buffer);
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
    alSourcef(source, AL_GAIN, max(volume, 0.0));
    alSourcef(source, AL_PITCH, speed);
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    alSourcePlay(source);
    return channel;
}

struct Gosu::Song::Impl
{
    unique_ptr<AudioFile> file;
    ALuint buffers[2];
    double volume;

    bool stream_to_buffer(ALuint buffer)
    {
        #ifdef GOSU_IS_IPHONE
        static const unsigned BUFFER_SIZE = 4096 * 4;
        #else
        static const unsigned BUFFER_SIZE = 4096 * 8;
        #endif
        char audio_data[BUFFER_SIZE];
        size_t read_bytes = file->read_data(audio_data, BUFFER_SIZE);
        if (read_bytes > 0) {
            alBufferData(buffer, file->format(), audio_data,
                         static_cast<ALsizei>(read_bytes), file->sample_rate());
        }
        return read_bytes > 0;
    }
};

Gosu::Song::Song(const string& filename)
: pimpl(new Impl)
{
    if (is_ogg_file(filename)) {
        File source_file(filename);
        pimpl->file.reset(new OggFile(source_file.front_reader()));
    }
    else {
    #ifdef GOSU_IS_MAC
        pimpl->file.reset(new AudioToolboxFile(filename));
    #else
        try {
            pimpl->file.reset(new SndFile(filename));
        }
        catch (const runtime_error& ex) {
            File source_file(filename);
            pimpl->file.reset(new MPEGFile(source_file.front_reader()));
        }
    #endif
    }

    al_initialize();
    alGenBuffers(2, pimpl->buffers);
}

Gosu::Song::Song(Reader reader)
: pimpl(new Impl)
{
    if (is_ogg_file(reader)) {
        pimpl->file.reset(new OggFile(reader));
    }
    else {
    #ifdef GOSU_IS_MAC
        pimpl->file.reset(new AudioToolboxFile(reader));
    #else
        try {
            pimpl->file.reset(new SndFile(reader));
        }
        catch (const runtime_error& ex) {
            pimpl->file.reset(new MPEGFile(reader));
        }
    #endif
    }

    al_initialize();
    alGenBuffers(2, pimpl->buffers);
}

Gosu::Song::~Song()
{
    // It's hard to free things in the right order in Ruby/Gosu.
    // Make sure buffers aren't deleted after the context/device are shut down.
    if (!al_initialized()) return;

    stop();

    alDeleteBuffers(2, pimpl->buffers);
}

Gosu::Song* Gosu::Song::current_song()
{
    return cur_song;
}

void Gosu::Song::play(bool looping)
{
    if (paused()) {
        alSourcePlay(al_source_for_songs());
    }
    
    if (cur_song && cur_song != this) {
        cur_song->stop();
        assert (cur_song == nullptr);
    }
    
    if (cur_song == nullptr) {
        ALuint source = al_source_for_songs();

        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSourcef(source, AL_GAIN, volume());
        alSourcef(source, AL_PITCH, 1);
        alSourcei(source, AL_LOOPING, AL_FALSE); // need to implement this manually...

        pimpl->stream_to_buffer(pimpl->buffers[0]);
        pimpl->stream_to_buffer(pimpl->buffers[1]);

        // TODO: Not good for songs with less than two buffers full of data.
        alSourceQueueBuffers(source, 2, pimpl->buffers);
        alSourcePlay(source);
    }
    
    cur_song = this;
    cur_song_looping = looping;
}

void Gosu::Song::pause()
{
    if (cur_song == this) {
        alSourcePause(al_source_for_songs());
    }
}

bool Gosu::Song::paused() const
{
    if (cur_song != this) return false;

    ALint state;
    alGetSourcei(al_source_for_songs(), AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

void Gosu::Song::stop()
{
    if (cur_song != this) return;
    
    ALuint source = al_source_for_songs();

    alSourceStop(source);

    // Unqueue all buffers for this source.
    // The number of QUEUED buffers apparently includes the number of PROCESSED ones,
    // so getting rid of the QUEUED ones is enough.
    ALuint buffer;
    int queued;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
    while (queued--) {
        alSourceUnqueueBuffers(source, 1, &buffer);
    }

    pimpl->file->rewind();

    cur_song = nullptr;
}

bool Gosu::Song::playing() const
{
    return cur_song == this && !paused();
}

double Gosu::Song::length() const
{
    return pimpl->file->length();
}

double Gosu::Song::volume() const
{
    return pimpl->volume;
}

void Gosu::Song::set_volume(double volume)
{
    pimpl->volume = clamp(volume, 0.0, 1.0);
    alSourcef(al_source_for_songs(), AL_GAIN, pimpl->volume);
}

void Gosu::Song::update()
{
    if (cur_song == nullptr) return;

    ALuint source = al_source_for_songs();

    ALuint buffer;
    int processed;
    bool active = true;

    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    for (int i = 0; i < processed; ++i) {
        alSourceUnqueueBuffers(source, 1, &buffer);
        active = cur_song->pimpl->stream_to_buffer(buffer);
        if (active) alSourceQueueBuffers(source, 1, &buffer);
    }

    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (active && state != AL_PLAYING && state != AL_PAUSED) {
        // We apparently got starved because this method wasn't being called in time - resume
        // playback and hope nobody will notice.
        alSourcePlay(source);
    }
    else if (!active) {
        // We got starved and there is nothing left to play.
        cur_song->stop();

        if (cur_song_looping) {
            // Start anew.
            cur_song->play(true);
        }
        else {
            // Let the world know the song is over.
            cur_song = nullptr;
        }
    }
}
