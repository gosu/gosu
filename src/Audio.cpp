#include <Gosu/Audio.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Math.hpp>
#include "AudioFile.hpp"
#include "AudioImpl.hpp"
#include <algorithm>
#include <cassert>

// These are global variables and not even thread-safe at that, but Gosu::Song is a legacy construct
// that will be replaced by a unified Sound class anyway in #562.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static Gosu::Song* cur_song = nullptr;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool cur_song_looping;

struct Gosu::Sample::Impl : private Gosu::Noncopyable
{
    ALuint buffer;

    explicit Impl(AudioFile&& audio_file)
    : buffer{}
    {
        al_initialize();
        alGenBuffers(1, &buffer);
        alBufferData(buffer, audio_file.format(), &audio_file.decoded_data().front(),
                     static_cast<ALsizei>(audio_file.decoded_data().size()),
                     static_cast<ALsizei>(audio_file.sample_rate()));
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

Gosu::Sample::Sample(const std::string& filename)
: m_impl{new Impl(AudioFile(filename))}
{
}

Gosu::Sample::Sample(Gosu::Reader reader)
: m_impl{new Impl(AudioFile(reader))}
{
}

Gosu::Channel Gosu::Sample::play(double volume, double speed, bool looping) const
{
    return play_pan(0.0, volume, speed, looping);
}

Gosu::Channel Gosu::Sample::play_pan(double pan, double volume, double speed, bool looping) const
{
    if (!m_impl) return Channel{};

    Channel channel = allocate_channel();

    // Couldn't allocate a free channel.
    if (channel.current_channel() == NO_CHANNEL) return channel;

    ALuint source = al_source_for_channel(channel.current_channel());
    alSourcei(source, AL_BUFFER, static_cast<ALint>(m_impl->buffer));
    alSource3f(source, AL_POSITION, static_cast<ALfloat>(pan * 10), 0, 0);
    alSourcef(source, AL_GAIN, static_cast<ALfloat>(std::max(volume, 0.0)));
    alSourcef(source, AL_PITCH, static_cast<ALfloat>(speed));
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    alSourcePlay(source);
    return channel;
}

// AudioFile impl
struct Gosu::Song::Impl : private Gosu::Noncopyable
{
private:
    double m_volume = 1.0;
    std::unique_ptr<AudioFile> m_file;
    ALuint m_buffers[2];

    void apply_volume()
    {
        alSourcef(al_source_for_songs(), AL_GAIN, static_cast<ALfloat>(std::max(volume(), 0.0)));
    }

    bool stream_to_buffer(ALuint buffer)
    {
        char audio_data[4096 * 8];
        size_t read_bytes = m_file->read_data(audio_data, sizeof audio_data);
        if (read_bytes > 0) {
            alBufferData(buffer, m_file->format(), audio_data, static_cast<ALsizei>(read_bytes),
                         static_cast<ALsizei>(m_file->sample_rate()));
        }
        return read_bytes > 0;
    }

public:
    explicit Impl(const std::string& filename)
    : m_buffers{},
      m_file{new AudioFile{filename}}
    {
        al_initialize();
        alGenBuffers(2, m_buffers);
    }

    explicit Impl(Reader reader)
    : m_buffers{},
      m_file{new AudioFile{reader}}
    {
        al_initialize();
        alGenBuffers(2, m_buffers);
    }

    ~Impl()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffers aren't deleted after the context/device are shut down.
        if (!al_initialized()) return;

        alDeleteBuffers(2, m_buffers);
    }

    void play()
    {
        ALuint source = al_source_for_songs();

        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSourcef(source, AL_GAIN, static_cast<ALfloat>(std::max(volume(), 0.0)));
        alSourcef(source, AL_PITCH, 1);
        alSourcei(source, AL_LOOPING, AL_FALSE); // need to implement this manually...

        stream_to_buffer(m_buffers[0]);
        stream_to_buffer(m_buffers[1]);

        // TODO: Not good for songs with less than two buffers full of data.
        alSourceQueueBuffers(source, 2, m_buffers);
        alSourcePlay(source);
    }

    void stop()
    {
        ALuint source = al_source_for_songs();

        alSourceStop(source);

        alSourcei(source, AL_BUFFER, AL_NONE);

        // Dequeue all buffers for this source.
        // The number of QUEUED buffers apparently includes the number of PROCESSED ones,
        // so getting rid of the QUEUED ones is enough.
        ALuint buffer;
        int queued;
        alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
        while (queued--) {
            alSourceUnqueueBuffers(source, 1, &buffer);
        }

        m_file->rewind();
    }

    static void pause() //
    {
        alSourcePause(al_source_for_songs());
    }

    static void resume() //
    {
        alSourcePlay(al_source_for_songs());
    }

    static bool paused()
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
                play();
            }
            else {
                // Let the world know we're finished.
                cur_song = nullptr;
            }
        }
    }

    double volume() const
    {
        return m_volume;
    }

    void set_volume(double volume)
    {
        m_volume = std::clamp(volume, 0.0, 1.0);
        apply_volume();
    }
};

Gosu::Song::Song(const std::string& filename)
: m_impl{new Impl(filename)}
{
}

Gosu::Song::Song(Reader reader)
: m_impl{new Impl(reader)}
{
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
        m_impl->resume();
    }

    if (cur_song && cur_song != this) {
        cur_song->stop();
        assert(cur_song == nullptr);
    }

    if (cur_song == nullptr) {
        m_impl->play();
    }

    cur_song = this;
    cur_song_looping = looping;
}

void Gosu::Song::pause()
{
    if (cur_song == this) {
        m_impl->pause();
    }
}

bool Gosu::Song::paused() const
{
    return cur_song == this && m_impl->paused();
}

void Gosu::Song::stop()
{
    if (cur_song == this) {
        m_impl->stop();
        cur_song = nullptr;
    }
}

bool Gosu::Song::playing() const
{
    return cur_song == this && !m_impl->paused();
}

double Gosu::Song::volume() const
{
    return m_impl->volume();
}

void Gosu::Song::set_volume(double volume)
{
    m_impl->set_volume(volume);
}

void Gosu::Song::update()
{
    if (current_song()) {
        current_song()->m_impl->update();
    }
}
