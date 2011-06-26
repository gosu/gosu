//! \file Audio.hpp
//! Contains all the classes of Gosu's audio system.

#ifndef GOSU_AUDIO_HPP
#define GOSU_AUDIO_HPP

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <Gosu/Fwd.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/TR1.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    // Deprecated.
    #ifndef SWIG
    GOSU_DEPRECATED class Audio;
    #endif

    //! An instance of a Sample playing. Can be used to stop sounds dynamically,
    //! or to check if they are finished.
    //! It is recommended that you throw away sample instances if possible,
    //! as they could accidentally refer to other sounds being played after
    //! a very long time has passed.
	class SampleInstance
	{
		int handle, extra;
        bool alive() const;		

	public:
        //! Called by Sample, do not use.
		SampleInstance(int handle, int extra);

		bool playing() const;
		bool paused() const;
        //! Pauses this instance to be resumed afterwards. It will still keep a channel filled while paused.
        void pause();
        void resume();
        //! Stops this instance of a sound being played.
        //! Calling this twice, or too late, does not do any harm.
		void stop();

        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        void changeVolume(double volume);
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        void changePan(double pan);
        //! \param speed Playback speed is only limited by FMOD's
        //! capabilities and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        void changeSpeed(double speed);
	};

    //! A sample is a short sound that is completely loaded in memory, can be
    //! played multiple times at once and offers very flexible playback
    //! parameters. Use samples for everything that's not music.
    class Sample
    {
        struct SampleData;
        std::tr1::shared_ptr<SampleData> data;
    
    public:
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample from a file.
        explicit Sample(const std::wstring& filename);
        
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample data from a stream.
        explicit Sample(Reader reader);
        
        //! Plays the sample without panning.
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by the underlying audio library,
        //! and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        SampleInstance play(double volume = 1, double speed = 1,
            bool looping = false) const;

        //! Plays the sample with panning. Even if pan is 0.0, the sample will
        //! not be as loud as if it were played by calling play() due to the
        //! way the panning works.
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by by the underlying audio library,
        //! and can accept very high
        //! or low values. Use 1.0 for normal playback speed.
        SampleInstance playPan(double pan, double volume = 1, double speed = 1,
            bool looping = false) const;

        #ifndef SWIG
        GOSU_DEPRECATED Sample(Audio& audio, const std::wstring& filename);
        GOSU_DEPRECATED Sample(Audio& audio, Reader reader);
        #endif
    };

    //! Songs are less flexible than samples in that they can only be played
    //! one at a time and without panning or speed parameters.
    class Song
    {
        class BaseData;
        class ModuleData;
        class StreamData;
        Song(const Song&);
        Song& operator=(const Song&);
        std::auto_ptr<BaseData> data;
    
    public:
        //! Constructs a song that can be played on the provided audio system
        //! and loads the song from a file. The type is determined from the
        //! filename.
        explicit Song(const std::wstring& filename);
        
        //! Constructs a song of the specified type that can be played on the
        //! provided audio system and loads the song data from a stream.
        explicit Song(Reader reader);
        
        ~Song();
        
        //! Returns the song currently being played or paused, or 0 if
        //! no song has been played yet or the last song has finished
        //! playing.
        static Song* currentSong();
        
        //! Starts or resumes playback of the song. This will stop all other
        //! songs and set the current song to this object.
        void play(bool looping = false);
        //! Pauses playback of the song. It is not considered being played.
        //! currentSong will stay the same.
        void pause();
        //! Returns true if the song is the current song, but in paused
        //! mode.
        bool paused() const;
        //! Stops playback of this song if it is currently played or paused.
        //! Afterwards, currentSong will return 0.
        void stop();
        //! Returns true if the song is currently playing.
        bool playing() const;
        //! Returns the current volume of the song.
        double volume() const;
        //! Changes the volume of the song.
        void changeVolume(double volume);
        
        //! Called every tick by Window for management purposes.
        static void update();

        #ifndef SWIG
        enum Type { stStream, stModule };
        GOSU_DEPRECATED Song(Audio&, const std::wstring& filename);
        GOSU_DEPRECATED Song(Audio&, Type type, Reader reader);
        #endif
    };
}

#endif
