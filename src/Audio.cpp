#include "AudioImpl.hpp"
#include "AudioFile.hpp"

#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>

#include <cassert>
#include <cstdlib>
#include <algorithm>
using namespace std;

static Gosu::Song* cur_song = nullptr;
static bool cur_song_looping;

struct Gosu::Sample::Impl
{
    ALuint buffer;

    Impl(AudioFile&& audio_file)
    {
        al_initialize();
        alGenBuffers(1, &buffer);
        alBufferData(buffer, audio_file.format(), &audio_file.decoded_data().front(),
                     static_cast<ALsizei>(audio_file.decoded_data().size()),
                     audio_file.sample_rate());
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
    pimpl.reset(new Impl(AudioFile(filename)));
}

Gosu::Sample::Sample(Gosu::Reader reader)
{
    pimpl.reset(new Impl(AudioFile(reader)));
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

// AudioFile impl
struct Gosu::Song::Impl
{
    double volume_ = 1.0;
    unique_ptr<AudioFile> file;
    ALuint buffers[2];
    
    void apply_volume()
    {
        alSourcef(al_source_for_songs(), AL_GAIN, max(volume(), 0.0));
    }
    
    bool stream_to_buffer(ALuint buffer)
    {
        char audio_data[4096 * 8];
        size_t read_bytes = file->read_data(audio_data, sizeof audio_data);
        if (read_bytes > 0) {
            alBufferData(buffer, file->format(), audio_data,
                         static_cast<ALsizei>(read_bytes), file->sample_rate());
        }
        return read_bytes > 0;
    }
    
public:
    explicit Impl(const string& filename)
    {
        file.reset(new AudioFile(filename));

        al_initialize();
        alGenBuffers(2, buffers);
    }

    explicit Impl(Reader reader)
    {
        file.reset(new AudioFile(reader));

        al_initialize();
        alGenBuffers(2, buffers);
    }
    
    ~Impl()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffers aren't deleted after the context/device are shut down.
        if (!al_initialized()) return;
        
        alDeleteBuffers(2, buffers);
    }
    
    void play(bool looping)
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

    void stop()
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
    
    void pause()
    {
        alSourcePause(al_source_for_songs());
    }
    
    void resume()
    {
        alSourcePlay(al_source_for_songs());
    }
    
    bool paused() const
    {
        ALint state;
        alGetSourcei(al_source_for_songs(), AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }
    
    void update()
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

Gosu::Song::Song(const string& filename)
{
    pimpl.reset(new Impl(filename));
}

Gosu::Song::Song(Reader reader)
{
    pimpl.reset(new Impl(reader));
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
        pimpl->resume();
    }
    
    if (cur_song && cur_song != this) {
        cur_song->stop();
        assert (cur_song == nullptr);
    }
    
    if (cur_song == nullptr) {
        pimpl->play(looping);
    }
    
    cur_song = this;
    cur_song_looping = looping;
}

void Gosu::Song::pause()
{
    if (cur_song == this) {
        pimpl->pause();
    }
}

bool Gosu::Song::paused() const
{
    return cur_song == this && pimpl->paused();
}

void Gosu::Song::stop()
{
    if (cur_song == this) {
        pimpl->stop();
        cur_song = nullptr;
    }
}

bool Gosu::Song::playing() const
{
    return cur_song == this && !pimpl->paused();
}

double Gosu::Song::volume() const
{
    return pimpl->volume();
}

void Gosu::Song::set_volume(double volume)
{
    pimpl->set_volume(volume);
}

void Gosu::Song::update()
{
    if (current_song()) {
        current_song()->pimpl->update();
    }
}
