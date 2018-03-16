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
    //! Sound::play returns a Channel that represents the sound currently being played.
    //! This object can be used to stop sounds dynamically, or to check whether they have finished.
    class Channel
    {
        class BaseData;
        class ModuleData;
        class StreamData;
        std::unique_ptr<BaseData> data;

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

        //! Returns the current volume of the song.
        double volume() const;
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full volume).
        void set_volume(double volume);

        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        void set_pan(double pan);
        //! \param speed Use 1.0 for normal playback speed.
        void set_speed(double speed);
    };


    //! A sound handles all your audio needs from short samples (e.g. explosions) to long songs (e.g. background music)
    //!
    //! Use streaming = false for everything that's not music. (Maybe add a more detailed explanation why this is better,
    //! because some the disadvantages of song might dissappear with the merge?)
    class Sound
    {
        struct SoundData;
        std::shared_ptr<SoundData> data;
        std::vector<Channel> current_songs;
    public:
        //! Constructs an empty sample that acts as if the song had a length of 0.
        Sound();
        
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample from a file.
        explicit Sound(const std::string& filename, bool streaming = false);
        
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample data from a stream.
        explicit Sound(Reader reader, bool streaming = false);
        
        //! Plays the sample without panning.
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by the underlying audio library,
        //! and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        Channel play(double volume = 1, double speed = 1, bool looping = false, double pan = 0) const;

        //! Called every tick by Window for management purposes.
        static void update();
    };
}
