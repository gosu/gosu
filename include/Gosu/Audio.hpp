#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    /// Sample::play returns a Channel that represents the sound being played.
    /// This object can be used to stop or influence sounds as they are played, or to check whether
    /// playback has finished.
    class Channel
    {
        mutable int m_channel, m_token;

    public:
        /// This creates an "empty" Channel which is expired and cannot be resumed.
        Channel();
        /// For internal use only.
        Channel(int channel, int token);

        /// For internal use only.
        int current_channel() const;

        bool playing() const;
        bool paused() const;
        /// Pauses this instance to be resumed afterwards.
        /// Avoid leaving samples paused for too long, as they will still occupy one of Gosu's
        /// limited channels.
        void pause();
        void resume();
        /// Stops this m_channel if the sample is still being played.
        /// If this method is called when playback has finished, it has no effect.
        void stop();

        /// @param volume Can be anything from 0.0 (silence) to 1.0 (full volume).
        void set_volume(double volume);
        /// @param pan Can be anything from -1.0 (left) to 1.0 (right).
        void set_pan(double pan);
        /// @param speed Use 1.0 for normal playback speed.
        void set_speed(double speed);
    };

    /// A sample is a short sound that is completely loaded in memory, can be played multiple times
    /// at once, and offers very flexible playback parameters.
    class Sample
    {
        struct Impl;
        std::shared_ptr<Impl> m_impl;

    public:
        /// Constructs an empty sample that is inaudible when played.
        Sample();

        /// Loads a sample from a file.
        explicit Sample(const std::string& filename);

        /// Loads a sample from memory.
        explicit Sample(Buffer buffer);

        /// Plays the sample without panning.
        /// @param volume Can be anything from 0.0 (silence) to 1.0 (full volume).
        /// @param speed Playback speed is only limited by the underlying audio library,
        ///              and can accept very high or low values. Use 1.0 for normal playback speed.
        Channel play(double volume = 1.0, double speed = 1.0, bool looping = false) const;

        /// Plays the sample with panning. Even if pan is 0.0, the sample will
        /// not be as loud as if it were played by calling play() due to the
        /// way the panning works.
        /// @param pan Can be anything from -1.0 (left) to 1.0 (right).
        /// @param volume Can be anything from 0.0 (silence) to 1.0 (full volume).
        /// @param speed Playback speed is only limited by by the underlying audio library,
        ///              and can accept very high or low values. Use 1.0 for normal playback speed.
        Channel play_pan(double pan, double volume = 1.0, double speed = 1.0,
                         bool looping = false) const;
    };

    /// Songs are less flexible than Samples. Only one Song can be played at any given time,
    /// and there is no way to control its pan (stereo position) or speed.
    class Song : private Noncopyable
    {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        /// Loads or streams a song from a file.
        explicit Song(const std::string& filename);

        /// Loads or streams a song from memory.
        explicit Song(Buffer buffer);

        ~Song();

        /// Returns the song currently being played or paused, or 0 if no song has been played yet
        /// or the last song has finished playing.
        static Song* current_song();

        /// Starts or resumes playback of the song. This will stop all other songs and set the
        /// current song to this object.
        void play(bool looping = false);
        /// Pauses playback of the song. Song::current_song() will still refer to this song, but
        /// playing() will return false.
        void pause();
        /// Returns true if the song is the current song, but in paused mode.
        bool paused() const;
        /// Stops playback of this song if it is currently played or paused.
        /// Afterwards, current_song will return nullptr.
        void stop();
        /// Returns true if the song is currently playing.
        bool playing() const;
        /// Returns the current volume of the song.
        double volume() const;
        /// @param volume Can be anything from 0.0 (silence) to 1.0 (full volume).
        void set_volume(double volume);

        /// Called every tick by Window to feed new audio data to OpenAL.
        static void update();
    };
}
