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
#define WAVE_FILE AudioToolboxFile
#else
#include "MPEGFile.hpp"
#include "SndFile.hpp"
#define WAVE_FILE SndFile
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

struct Gosu::Sample::SampleData
{
    ALuint buffer;

    SampleData(AudioFile& audio_file)
    {
        al_initialize();
        alGenBuffers(1, &buffer);
        alBufferData(buffer, audio_file.format(), &audio_file.decoded_data().front(),
                     static_cast<ALsizei>(audio_file.decoded_data().size()),
                     audio_file.sample_rate());
    }
    
    ~SampleData()
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
        OggFile ogg_file(file.front_reader());
        data.reset(new SampleData(ogg_file));
    }
    else {
        WAVE_FILE audio_file(filename);
        data.reset(new SampleData(audio_file));
    }
}

Gosu::Sample::Sample(Gosu::Reader reader)
{
    if (is_ogg_file(reader)) {
        OggFile ogg_file(reader);
        data.reset(new SampleData(ogg_file));
        return;
    }

    try {
        WAVE_FILE audio_file(reader);
        data.reset(new SampleData(audio_file));
    }
    catch (const runtime_error& ex) {
    #ifndef GOSU_IS_MAC
        if (string(ex.what()).find("unknown format") != string::npos) {
            MPEGFile mpeg_file(reader);
            data.reset(new SampleData(mpeg_file));
            return;
        }
    #endif
        throw ex;
    }
}

Gosu::Channel Gosu::Sample::play(double volume, double speed, bool looping) const
{
    return play_pan(0, volume, speed, looping);
}

Gosu::Channel Gosu::Sample::play_pan(double pan, double volume, double speed, bool looping) const
{
    if (!data) return Channel(NO_CHANNEL, 0);

    Channel channel = allocate_channel();
    
    // Couldn't allocate a free channel.
    if (channel.current_channel() == NO_CHANNEL) return channel;
    
    ALuint source = al_source_for_channel(channel.current_channel());
    alSourcei(source, AL_BUFFER, data->buffer);
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
    alSourcef(source, AL_GAIN, max(volume, 0.0));
    alSourcef(source, AL_PITCH, speed);
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    alSourcePlay(source);
    return channel;
}

class Gosu::Song::BaseData
{
    BaseData(const BaseData&) = delete;
    BaseData& operator=(const BaseData&) = delete;
    
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
    
    void set_volume(double volume)
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
    ModuleData(const string& filename)
    {
        NSURL* URL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
        player = [[AVAudioPlayer alloc] initWithContentsOfURL:URL error:nil];
    }
    
    void play(bool looping) override
    {
        if (paused()) {
            stop();
        }
        player.numberOfLoops = looping ? -1 : 0;
        [player play];
    }
    
    void pause() override
    {
        [player pause];
    }
    
    void resume() override
    {
        [player play];
    }
    
    bool paused() const override
    {
        return !player.playing;
    };
    
    void stop() override
    {
        [player stop];
        player.currentTime = 0;
    }
    
    void update() override
    {
    }
};
#endif

// AudioFile impl
class Gosu::Song::StreamData : public BaseData
{
    unique_ptr<AudioFile> file;
    ALuint buffers[2];
    
    void apply_volume() override
    {
        alSourcef(al_source_for_songs(), AL_GAIN, max(volume(), 0.0));
    }
    
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
    
public:
    StreamData(const string& filename)
    {
        if (is_ogg_file(filename)) {
            File source_file(filename);
            file.reset(new OggFile(source_file.front_reader()));
        }
        else {
            try {
                file.reset(new WAVE_FILE(filename));
            }
            catch (const runtime_error& ex) {
            #ifndef GOSU_IS_MAC
                if (string(ex.what()).find("unknown format") != string::npos) {
                    File source_file(filename);
                    file.reset(new MPEGFile(source_file.front_reader()));
                }
                else
            #endif
                throw ex;
            }
        }
        
        al_initialize();
        alGenBuffers(2, buffers);
    }

    StreamData(Reader reader)
    {
        if (is_ogg_file(reader)) {
            file.reset(new OggFile(reader));
        }
        else {
            try {
                file.reset(new WAVE_FILE(reader));
            }
            catch (const runtime_error& ex) {
            #ifndef GOSU_IS_MAC
                if (string(ex.what()).find("unknown format") != string::npos) {
                    file.reset(new MPEGFile(reader));
                }
                else
            #endif
                throw ex;
            }
        }
        
        al_initialize();
        alGenBuffers(2, buffers);
    }
    
    ~StreamData()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffers aren't deleted after the context/device are shut down.
        if (!al_initialized()) return;
        
        alDeleteBuffers(2, buffers);
    }
    
    void play(bool looping) override
    {
        ALuint source = al_source_for_songs();

        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSourcef(source, AL_GAIN, max(volume(), 0.0));
        alSourcef(source, AL_PITCH, 1);
        alSourcei(source, AL_LOOPING, AL_FALSE); // need to implement this manually...

        stream_to_buffer(buffers[0]);
        stream_to_buffer(buffers[1]);
        
        // TODO: Not good for songs with less than two buffers full of data.
        alSourceQueueBuffers(source, 2, buffers);
        alSourcePlay(source);
    }

    void stop() override
    {
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
        
        file->rewind();
    }
    
    void pause() override
    {
        alSourcePause(al_source_for_songs());
    }
    
    void resume() override
    {
        alSourcePlay(al_source_for_songs());
    }
    
    bool paused() const override
    {
        ALint state;
        alGetSourcei(al_source_for_songs(), AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }
    
    void update() override
    {
        ALuint source = al_source_for_songs();

        ALuint buffer;
        int processed;
        bool active = true;
        
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        for (int i = 0; i < processed; ++i) {
            alSourceUnqueueBuffers(source, 1, &buffer);
            active = stream_to_buffer(buffer);
            if (active) alSourceQueueBuffers(source, 1, &buffer);
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

Gosu::Song::Song(const string& filename)
{
#ifdef GOSU_IS_IPHONE
    if (has_extension(filename, ".mp3") ||
            has_extension(filename, ".aac") ||
            has_extension(filename, ".m4a")) {
        data.reset(new ModuleData(filename));
    }
    else
#endif
    data.reset(new StreamData(filename));
}

Gosu::Song::Song(Reader reader)
{
    data.reset(new StreamData(reader));
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
        data->pause();
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

void Gosu::Song::set_volume(double volume)
{
    data->set_volume(volume);
}

void Gosu::Song::update()
{
    if (current_song()) {
        current_song()->data->update();
    }
}
