//! \file Audio.hpp
//! Contains all the classes of Gosu's audio system.

// REDESIGN: Rename all changeXXX() member functions to setXXX().
// REDESIGN: Don't require window argument, is this possible?

#ifndef GOSU_AUDIO_HPP
#define GOSU_AUDIO_HPP

#ifdef WIN32
#include <windows.h>
#endif
#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <string>

namespace Gosu
{
    //! Manages initialization and finalization of audio libraries. Must be
    //! created before and destroyed after all samples and songs.
    //! There can only be one instance of Gosu::Audio.
    //! Usually created internally by Gosu::Window.
    class Audio : boost::noncopyable
    {
    public:
#ifdef GOSU_IS_WIN
        explicit Audio(HWND window);
#else
        Audio();
#endif
        ~Audio();
    };

    //! An instance of a Sample playing. Can be used to stop sounds dynamically,
    //! or to check if they are finished.
    //! It is recommended that you throw away sample instances if possible,
    //! as they could accidentally refer to sounds played very long ago.
	class SampleInstance
	{
		int handle, extra;
		
	public:
        //! Called by Sample, do not use.
		SampleInstance(int handle, int extra);

		bool playing() const;
		void stop();

        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        void changeVolume(double volume);
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        void changePan(double pan);
        //! \param speed Playback speed is only limited by FMOD's
        //! capatibilities and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        void changeSpeed(double speed);
	};

    //! A sample is a short sound that is completely loaded in memory, can be
    //! played multiple times at once and offers very flexible playback
    //! parameters. In short, use samples for everything that's not music.
    class Sample
    {
        struct SampleData;
        boost::scoped_ptr<SampleData> data;

    public:
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample from a file.
        Sample(Audio& audio, const std::wstring& filename);
        
        //! Constructs a sample that can be played on the specified audio
        //! system and loads the sample data from a stream.
        Sample(Audio& audio, Reader reader);
        
        ~Sample();

        //! Plays the sample without panning.
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by FMOD's
        //! capatibilities and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        SampleInstance play(double volume = 1, double speed = 1) const;

        //! Plays the sample with panning. Even if pan is 0.0, the sample will
        //! not be as loud as if it were played by calling play() due to the
        //! way FMOD's panning works.
        //! \param pan Can be anything from -1.0 (left) to 1.0 (right).
        //! \param volume Can be anything from 0.0 (silence) to 1.0 (full
        //! volume).
        //! \param speed Playback speed is only limited by FMOD's
        //! capatibilities and can accept very high or low values. Use 1.0 for
        //! normal playback speed.
        SampleInstance playPan(double pan, double volume = 1, double speed = 1) const;
    };

    //! Songs are less flexible than samples in that they can only be played
    //! one at a time and without panning or speed parameters.
    class Song
    {
        class BaseData;
        class ModuleData;
        class StreamData;
        boost::scoped_ptr<BaseData> data;

    public:
        //! There are two types of songs that can be loaded as a Song: Streamed
        //! songs (like OGG) and modules (like MOD or XM).
        enum Type
        {
            stStream,
            stModule
        };

        //! Constructs a song that can be played on the provided audio system
        //! and loads the song from a file. The type is determined from the
        //! filename.
        Song(Audio&, const std::wstring& filename);
        
        //! Constructs a song of the specified type that can be played on the
        //! provided audio system and loads the song data from a stream.
        Song(Audio&, Type type, Reader reader);
        
        ~Song();

        //! Starts playback of the song. This will stop all other songs and
        //! cause the currently played song to restart if called on the
        //! current song.
        void play();
        //! Stops playback of this song if it is currently played.
        void stop();
        //! Returns if the song is currently playing.
        bool playing() const;
        //! Returns the current volume of the song.
        double volume() const;
        //! Changes the volume of the song.
        void changeVolume(double volume);
    };
}

#endif
