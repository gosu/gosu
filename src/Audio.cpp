#include "ALChannelManagement.hpp"
#include "OggFile.hpp"
#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#import <Foundation/Foundation.h>
#include "AudioToolboxFile.hpp"
#define WAVE_FILE AudioToolboxFile
#else
#include <AL/al.h>
#include <AL/alc.h>
#include "SndFile.hpp"
#define WAVE_FILE SndFile
#endif

#ifdef GOSU_IS_IPHONE
#import <AVFoundation/AVFoundation.h>
#endif

using namespace std;

namespace
{
    bool is_ogg_file(Gosu::Reader reader)
    {
        char magic_bytes[4];
        reader.read(magic_bytes, 4);
        return magic_bytes[0] == 'O' && magic_bytes[1] == 'g' &&
               magic_bytes[2] == 'g' && magic_bytes[3] == 'S';
    }

    bool is_ogg_file(const string& filename)
    {
        Gosu::File file(filename);
        return is_ogg_file(file.front_reader());
    }
    
    Gosu::Song* cur_song = nullptr;
    bool cur_song_looping;
}

#ifdef GOSU_IS_MAC
    #define CONSTRUCTOR_BEGIN \
        @autoreleasepool { \
        if (!al_channel_management.get()) \
            al_channel_management.reset(new ALChannelManagement)
    #define CONSTRUCTOR_END \
        }
#else
    #define CONSTRUCTOR_BEGIN \
        if (!al_channel_management.get()) \
            al_channel_management.reset(new ALChannelManagement)
    #define CONSTRUCTOR_END
#endif

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return false;
    
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool Gosu::SampleInstance::paused() const
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return false;
    
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

void Gosu::SampleInstance::pause()
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return;
    
    alSourcePause(source);
}

void Gosu::SampleInstance::resume()
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return;
    
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED) {
        alSourcePlay(source);
    }
}

void Gosu::SampleInstance::stop()
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return;
    
    alSourceStop(source);
}

void Gosu::SampleInstance::change_volume(double volume)
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return;
    
    alSourcef(source, AL_GAIN, volume);
}

void Gosu::SampleInstance::change_pan(double pan)
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return;
    
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
}

void Gosu::SampleInstance::change_speed(double speed)
{
    ALuint source = al_channel_management->source_if_still_playing(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE) return;
    
    alSourcef(source, AL_PITCH, speed);
}

struct Gosu::Sample::SampleData
{
    ALuint buffer, source;

    SampleData(AudioFile& audio_file)
    {
        alGenBuffers(1, &buffer);
        alBufferData(buffer, audio_file.format(), &audio_file.decoded_data().front(),
                     static_cast<ALsizei>(audio_file.decoded_data().size()),
                     audio_file.sample_rate());
    }
    
    ~SampleData()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffer isn't deleted after the context/device are shut down.
        if (!al_channel_management.get()) {
            return;
        }

        alDeleteBuffers(1, &buffer);
    }

private:
    SampleData(const SampleData&);
    SampleData& operator=(const SampleData&);
};

Gosu::Sample::Sample(const std::string& filename)
{
    CONSTRUCTOR_BEGIN;

    if (is_ogg_file(filename)) {
        Gosu::Buffer buffer;
        Gosu::load_file(buffer, filename);
        OggFile ogg_file(buffer.front_reader());
        data.reset(new SampleData(ogg_file));
    }
    else {
        WAVE_FILE audio_file(filename);
        data.reset(new SampleData(audio_file));
    }
    
    CONSTRUCTOR_END;
}

Gosu::Sample::Sample(Reader reader)
{
    CONSTRUCTOR_BEGIN;

    if (is_ogg_file(reader)) {
        OggFile ogg_file(reader);
        data.reset(new SampleData(ogg_file));
    }
    else {
        WAVE_FILE audio_file(reader);
        data.reset(new SampleData(audio_file));
    }
    
    CONSTRUCTOR_END;
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed, bool looping) const
{
    return play_pan(0, volume, speed, looping);
}

Gosu::SampleInstance Gosu::Sample::play_pan(double pan, double volume, double speed,
    bool looping) const
{
    std::pair<int, int> channel_and_token = al_channel_management->reserve_channel();
    if (channel_and_token.first == ALChannelManagement::NO_FREE_CHANNEL) {
        return Gosu::SampleInstance(channel_and_token.first, channel_and_token.second);
    }
    
    ALuint source = al_channel_management->source_if_still_playing(channel_and_token.first,
                                                                   channel_and_token.second);
    assert (source != ALChannelManagement::NO_SOURCE);
    alSourcei(source, AL_BUFFER, data->buffer);
    // TODO: This is not the old panning behavior!
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
    alSourcef(source, AL_GAIN, volume);
    alSourcef(source, AL_PITCH, speed);
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    alSourcePlay(source);

    return Gosu::SampleInstance(channel_and_token.first, channel_and_token.second);
}

class Gosu::Song::BaseData
{
    BaseData(const BaseData&);
    BaseData& operator=(const BaseData&);
    
    double volume_;

protected:
    BaseData() : volume_(1) {}
    virtual void apply_volume() = 0;

public:
    virtual ~BaseData() {}
    virtual void play(bool looping) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual bool paused() const = 0;
    virtual void stop() = 0;
    virtual void update() = 0;
    
    double volume() const
    {
        return volume_;
    }
    
    void change_volume(double volume)
    {
        volume_ = clamp(volume, 0.0, 1.0);
        apply_volume();
    }
};

#ifdef GOSU_IS_IPHONE
// AVAudioPlayer impl
class Gosu::Song::ModuleData : public BaseData
{
    AVAudioPlayer* player;
    
    void apply_volume()
    {
        player.volume = volume();
    }
    
public:
    ModuleData(const std::string& filename)
    {
        NSURL* URL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
        player = [[AVAudioPlayer alloc] initWithContentsOfURL:URL error:nil];
    }
    
    void play(bool looping)
    {
        if (paused()) {
            stop();
        }
        player.numberOfLoops = looping ? -1 : 0;
        [player play];
    }
    
    void pause()
    {
        [player pause];
    }
    
    void resume()
    {
        [player play];
    }
    
    bool paused() const
    {
        return !player.playing;
    };
    
    void stop()
    {
        [player stop];
        player.currentTime = 0;
    }
    
    void update()
    {
    }
};
#endif

// AudioFile impl
class Gosu::Song::StreamData : public BaseData
{
    std::unique_ptr<AudioFile> file;
    ALuint buffers[2];
    
    void apply_volume()
    {
        int source = lookup_source();
        if (source != ALChannelManagement::NO_SOURCE) {
            alSourcef(source, AL_GAIN, volume());
        }
    }
    
    int lookup_source() const
    {
        return al_channel_management->source_for_songs();
    }
    
    bool stream_to_buffer(ALuint buffer)
    {
        #ifdef GOSU_IS_IPHONE
        static const unsigned BUFFER_SIZE = 4096 * 4;
        #else
        static const unsigned BUFFER_SIZE = 4096 * 8;
        #endif
        char audio_data[BUFFER_SIZE];
        std::size_t read_bytes = file->read_data(audio_data, BUFFER_SIZE);
        if (read_bytes > 0) {
            alBufferData(buffer, file->format(), audio_data,
                         static_cast<ALsizei>(read_bytes), file->sample_rate());
        }
        return read_bytes > 0;
    }
    
public:
    StreamData(const std::string& filename)
    {
        if (is_ogg_file(filename)) {
            Gosu::File source_file(filename);
            file.reset(new OggFile(source_file.front_reader()));
        }
        else {
            file.reset(new WAVE_FILE(filename));
        }
        alGenBuffers(2, buffers);
    }

    StreamData(Reader reader)
    {
        if (is_ogg_file(reader)) {
            file.reset(new OggFile(reader));
        }
        else {
            file.reset(new WAVE_FILE(reader));
        }
        alGenBuffers(2, buffers);
    }
    
    ~StreamData()
    {
        if (al_channel_management.get()) {
            alDeleteBuffers(2, buffers);
        }
    }
    
    void play(bool looping)
    {
        int source = lookup_source();
        if (source != ALChannelManagement::NO_SOURCE) {
            alSource3f(source, AL_POSITION, 0, 0, 0);
            alSourcef(source, AL_GAIN, volume());
            alSourcef(source, AL_PITCH, 1);
            alSourcei(source, AL_LOOPING, AL_FALSE); // need to implement this manually...

            stream_to_buffer(buffers[0]);
            stream_to_buffer(buffers[1]);
            
            // TODO: Not good for songs with less than two buffers full of data.
            
            alSourceQueueBuffers(source, 2, buffers);
            alSourcePlay(source);
        }
    }

    void stop()
    {
        int source = lookup_source();
        if (source != ALChannelManagement::NO_SOURCE) {
            alSourceStop(source);

            ALuint buffer;
            
            // The number of QUEUED buffers apparently includes the number of
            // PROCESSED ones, so getting rid of the QUEUED ones is enough.
            
            int queued;
            alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
            while (queued--) {
                alSourceUnqueueBuffers(source, 1, &buffer);
            }
        }
        file->rewind();
    }
    
    void pause()
    {
        int source = lookup_source();
        if (source != ALChannelManagement::NO_SOURCE) {
            alSourcePause(source);
        }
    }
    
    void resume()
    {
        int source = lookup_source();
        if (source != ALChannelManagement::NO_SOURCE) {
            alSourcePlay(source);
        }
    }
    
    bool paused() const
    {
        int source = lookup_source();
        if (source == ALChannelManagement::NO_SOURCE) {
            return false;
        }
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }
    
    void update()
    {
        int source = lookup_source();

        ALuint buffer;
        int processed;
        bool active = true;
        
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        for (int i = 0; i < processed; ++i) {
            alSourceUnqueueBuffers(source, 1, &buffer);
            active = stream_to_buffer(buffer);
            if (active) {
                alSourceQueueBuffers(source, 1, &buffer);
            }
        }
        
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (active && state != AL_PLAYING && state != AL_PAUSED) {
            // We seemingly got starved.
            alSourcePlay(source);
        }
        else if (!active) {
            // We got starved and there is nothing left to play.
            stop();

            if (cur_song_looping) {
                // Start anew.
                play(true);
            }
            else {
                // Let the world know we're finished.
                cur_song = nullptr;
            }
        }
    }
};

Gosu::Song::Song(const std::string& filename)
{
#ifdef GOSU_IS_IPHONE
    if (has_extension(filename, ".mp3") ||
            has_extension(filename, ".aac") ||
            has_extension(filename, ".m4a")) {
        CONSTRUCTOR_BEGIN;
        data.reset(new ModuleData(filename));
        CONSTRUCTOR_END;
        return;
    }
#endif

    CONSTRUCTOR_BEGIN;
    data.reset(new StreamData(filename));
    CONSTRUCTOR_END;
}

Gosu::Song::Song(Reader reader)
{
    CONSTRUCTOR_BEGIN;
    data.reset(new StreamData(reader));
    CONSTRUCTOR_END;
}

Gosu::Song::~Song()
{
    stop();
}

Gosu::Song* Gosu::Song::current_song()
{
    return cur_song;
}

void Gosu::Song::play(bool looping)
{
    if (paused()) {
        data->resume();
    }
    
    if (cur_song && cur_song != this) {
        cur_song->stop();
        assert (cur_song == nullptr);
    }
    
    if (cur_song == nullptr) {
        data->play(looping);
    }
    
    cur_song = this;
    cur_song_looping = looping;
}

void Gosu::Song::pause()
{
    if (cur_song == this) {
        data->pause(); // may be redundant
    }
}

bool Gosu::Song::paused() const
{
    return cur_song == this && data->paused();
}

void Gosu::Song::stop()
{
    if (cur_song == this) {
        data->stop();
        cur_song = nullptr;
    }
}

bool Gosu::Song::playing() const
{
    return cur_song == this && !data->paused();
}

double Gosu::Song::volume() const
{
    return data->volume();
}

void Gosu::Song::change_volume(double volume)
{
    data->change_volume(volume);
}

void Gosu::Song::update()
{
    if (current_song()) {
        current_song()->data->update();
    }
}
