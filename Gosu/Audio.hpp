//! \file Audio.hpp
//! Contains all the classes of Gosu's audio system.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    //! Sample::play returns a Channel that represents the sound currently being played.
    //! This object can be used to stop sounds dynamically, or to check whether they have finished.
    class Channel
    {
        mutable int channel, token;

    public:
        //! For internal use only.
        Channel(int channel, int token);
        
        int current_channel() const;
        
        bool playing() const;
        bool paused() const;
        //! Pauses this instance to be resumed afterwards.
        //! It will still occupy an audio channel while paused.
        void pause();
        void resume();
        //! Stops this instance of a sound being played.
        //! Calling this twice, or too late, does not do any harm.
        void stop();

        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full volume).
        void set_volume(double volume);
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        void set_pan(double pan);
        //! \param speed Use 1.0 for normal playback speed.
        void set_speed(double speed);
    };
    
    //! A sample is a short sound that is completely loaded in memory, can be
    //! played multiple times at once and offers very flexible playback
    //! parameters. Use samples for everything that's not music.
    class Sample
    {
        struct SampleData;
        std::shared_ptr<SampleData> data;

    public:
        //! Constructs an empty sample that acts as if the song had a length of 0.
        Sample();
        
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample from a file.
        explicit Sample(const std::string& filename);
        
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample data from a stream.
        explicit Sample(Reader reader);
        
        //! Plays the sample without panning.
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by the underlying audio library,
        //! and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        Channel play(double volume = 1, double speed = 1, bool looping = false) const;

        //! Plays the sample with panning. Even if pan is 0.0, the sample will
        //! not be as loud as if it were played by calling play() due to the
        //! way the panning works.
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by by the underlying audio library,
        //! and can accept very high
        //! or low values. Use 1.0 for normal playback speed.
        Channel play_pan(double pan, double volume = 1, double speed = 1,
            bool looping = false) const;
    };

    //! Songs are less flexible than samples. Only Song can be played at any given time,
    //! and there is no way to control its pan (stereo position) or speed.
    class Song
    {
        class BaseData;
        class ModuleData;
        class StreamData;
        std::unique_ptr<BaseData> data;
        
        // Non-movable to avoid dangling internal references.
        Song(Song&&) = delete;
        // Non-movable to avoid dangling internal references.
        Song& operator=(Song&&) = delete;

    public:
        //! Constructs a song that can be played on the provided audio system
        //! and loads the song from a file. The type is determined from the
        //! filename.
        explicit Song(const std::string& filename);
        
        //! Constructs a song of the specified type that can be played on the
        //! provided audio system and loads the song data from a stream.
        explicit Song(Reader reader);
        
        ~Song();
        
        //! Returns the song currently being played or paused, or 0 if
        //! no song has been played yet or the last song has finished
        //! playing.
        static Song* current_song();
        
        //! Starts or resumes playback of the song. This will stop all other
        //! songs and set the current song to this object.
        void play(bool looping = false);
        //! Pauses playback of the song. It is not considered being played.
        //! current_song will stay the same.
        void pause();
        //! Returns true if the song is the current song, but in paused
        //! mode.
        bool paused() const;
        //! Stops playback of this song if it is currently played or paused.
        //! Afterwards, current_song will return nullptr.
        void stop();
        //! Returns true if the song is currently playing.
        bool playing() const;
        //! Returns the current volume of the song.
        double volume() const;
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        void set_volume(double volume);
        
        //! Called when the song finished (doesn't get fired on pause/stop)
        virtual void finished() {}

        //! Called every tick by Window for management purposes.
        static void update();
    };
}
