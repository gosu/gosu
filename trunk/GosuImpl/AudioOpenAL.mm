#include <GosuImpl/MacUtility.hpp>
#include <GosuImpl/Audio/AudioFileMac.hpp>
#include <GosuImpl/Audio/ALChannelManagement.hpp>

#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#if __LP64__ || NS_BUILD_32_LIKE_64
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif

using namespace std;

namespace
{
    using namespace Gosu;
    
    /*GOSU_NORETURN void throwLastALError(const char* action)
    {
        string message = "OpenAL error " +
                         boost::lexical_cast<string>(alcGetError(alDevice));
        if (action)
            message += " while ", message += action;
        throw runtime_error(message);
    }

    inline void alCheck(const char* action = 0)
    {
        if (alcGetError(alDevice) != ALC_NO_ERROR)
            throwLastALError(action);
    }*/

    Song* curSong = 0;
}

Gosu::Audio::Audio()
{
    if (alChannelManagement)
        throw std::logic_error("Multiple Gosu::Audio instances not supported");
    alChannelManagement.reset(new ALChannelManagement);
}

Gosu::Audio::~Audio()
{
    alChannelManagement.reset();
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    return false;
}

bool Gosu::SampleInstance::paused() const
{
    return false;
}

void Gosu::SampleInstance::pause()
{
}

void Gosu::SampleInstance::resume()
{
}

void Gosu::SampleInstance::stop()
{
}

void Gosu::SampleInstance::changeVolume(double volume)
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourcef(source, AL_GAIN, volume);
}

void Gosu::SampleInstance::changePan(double pan)
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    // TODO: This is not the old panning behavior!
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
}

void Gosu::SampleInstance::changeSpeed(double speed)
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourcef(source, AL_PITCH, speed);
}

struct Gosu::Sample::SampleData : boost::noncopyable
{
    NSUInteger buffer, source;

    SampleData(const AudioFile& audioFile)
    {
        alGenBuffers(1, &buffer);
        alBufferData(buffer,
                     audioFile.getFormatAndSampleRate().first,
                     &audioFile.getDecodedData().front(),
                     audioFile.getDecodedData().size(),
                     audioFile.getFormatAndSampleRate().second);
    }

    ~SampleData()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffer isn't deleted after the context/device are shut down.
        
        if (!alChannelManagement)
            return;
            
        alDeleteBuffers(1, &buffer);
    }
};

Gosu::Sample::Sample(Audio& audio, const std::wstring& filename)
{
    AudioFile audioFile(filename);
    data.reset(new SampleData(audioFile));
}

Gosu::Sample::Sample(Audio& audio, Reader reader)
{
    throw logic_error("NYI");
}

Gosu::Sample::~Sample()
{
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed,
    bool looping) const
{
    return playPan(0, volume, speed, looping);
}

Gosu::SampleInstance Gosu::Sample::playPan(double pan, double volume,
    double speed, bool looping) const
{
    std::pair<int, int> channelAndToken = alChannelManagement->reserveChannel();
    if (channelAndToken.first == ALChannelManagement::NO_FREE_CHANNEL)
        return Gosu::SampleInstance(channelAndToken.first, channelAndToken.second);
        
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(channelAndToken.first,
                                                                  channelAndToken.second);
    assert(source != ALChannelManagement::NO_SOURCE);
    alSourcei(source, AL_BUFFER, data->buffer);
    // TODO: This is not the old panning behavior!
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
    alSourcef(source, AL_GAIN, volume);
    alSourcef(source, AL_PITCH, speed);
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    alSourcePlay(source);

    return Gosu::SampleInstance(channelAndToken.first, channelAndToken.second);
}

class Gosu::Song::BaseData : boost::noncopyable
{
//    double volume_;
//
//protected:
//    BaseData() : volume_(1) {}
//    virtual void applyVolume() = 0;
//
//public:
//    virtual ~BaseData() {}
//
//    virtual void play(bool looping) = 0;
//    virtual void pause() = 0;
//    virtual bool paused() const = 0;
//    virtual void stop() = 0;
//    
//    double volume() const
//    {
//        return volume_;
//    }
//    
//    void changeVolume(double volume)
//    {
//        volume_ = clamp(volume, 0.0, 1.0);
//        applyVolume();
//    }
};

//class Gosu::Song::StreamData : public BaseData
//{
//    FSOUND_STREAM* stream;
//    int handle;
//    std::vector<char> buffer;
//
//    static signed char F_CALLBACKAPI endSongCallback(FSOUND_STREAM*, void*,
//        int, void* self)
//    {
//        curSong = 0;
//        static_cast<StreamData*>(self)->handle = -1;
//        return 0;
//    }
//
//public:
//    StreamData(Gosu::Reader reader)
//    : stream(0), handle(-1)
//    {
//        buffer.resize(reader.resource().size() - reader.position());
//        reader.read(&buffer[0], buffer.size());
//
//// Disabled for licensing reasons.
//// If you have a license to play MP3 files, compile with GOSU_ALLOW_MP3.
//#ifndef GOSU_ALLOW_MP3
//        if (buffer.size() > 2 &&
//            ((buffer[0] == '\xff' && (buffer[1] & 0xfe) == '\xfa') ||
//            (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3')))
//        {
//            throw std::runtime_error("MP3 file playback not allowed");
//        }
//#endif
//
//        stream = FSOUND_Stream_Open(&buffer[0], FSOUND_LOADMEMORY | FSOUND_LOOP_NORMAL,
//            0, buffer.size());
//        if (stream == 0)
//            throwLastFMODError();
//        
//        FSOUND_Stream_SetEndCallback(stream, endSongCallback, this);
//    }
//    
//    ~StreamData()
//    {
//        // TODO: Should be checked for earlier, as play would crash too.
//        // This is just because Ruby's GC will free objects in a weird
//        // order.
//        if (!fmodInitialized)
//            return;
//
//        if (stream != 0)
//            FSOUND_Stream_Close(stream);
//    }
//    
//    void play(bool looping)
//    {
//        if (handle == -1)
//        {
//            handle = FSOUND_Stream_Play(FSOUND_FREE, stream);
//                FSOUND_Stream_SetLoopCount(stream, looping ? -1 : 0);
//        }
//        else if (paused())
//            FSOUND_SetPaused(handle, 0);
//        applyVolume();
//    }
//    
//    void pause()
//    {
//        if (handle != -1)
//            FSOUND_SetPaused(handle, 1);
//    }
//    
//    bool paused() const
//    {
//        return handle != -1 && FSOUND_GetPaused(handle);
//    }
//    
//    void stop()
//    {
//        fmodCheck(FSOUND_Stream_Stop(stream));
//        handle = -1; // The end callback is NOT being called!
//    }
//    
//    void applyVolume()
//    {
//        if (handle != -1)
//            FSOUND_SetVolume(handle, static_cast<int>(volume() * 255));
//    }
//};
//
//class Gosu::Song::ModuleData : public Gosu::Song::BaseData
//{
//    FMUSIC_MODULE* module_;
//
//public:
//    ModuleData(Reader reader)
//    : module_(0)
//    {
//        std::vector<char> buffer(reader.resource().size() - reader.position());
//        reader.read(&buffer[0], buffer.size());
//
//        module_ = FMUSIC_LoadSongEx(&buffer[0], 0, buffer.size(),
//            FSOUND_LOADMEMORY | FSOUND_LOOP_OFF, 0, 0);
//        if (module_ == 0)
//            throwLastFMODError();
//    }
//
//    ~ModuleData()
//    {
//        // TODO: Should be checked for earlier, as play would crash too.
//        // This is just because Ruby's GC will free objects in a weird
//        // order.
//        if (!fmodInitialized)
//            return;
//
//        if (module_ != 0)
//            FMUSIC_FreeSong(module_);
//    }
//
//    void play(bool looping)
//    {
//        if (paused())
//            FMUSIC_SetPaused(module_, 0);
//        else
//            FMUSIC_PlaySong(module_);
//        FMUSIC_SetLooping(module_, looping);
//        applyVolume();
//    }
//    
//    void pause()
//    {
//        FMUSIC_SetPaused(module_, 1);
//    }
//    
//    bool paused() const
//    {
//        return FMUSIC_GetPaused(module_);
//    }
//
//    void stop()
//    {
//        fmodCheck(FMUSIC_StopSong(module_));
//        FMUSIC_SetPaused(module_, false);
//    }
//    
//    void applyVolume()
//    {
//        // Weird as it may seem, the FMOD doc really says volume can
//        // be 0 to 256, *inclusive*, for this function.
//        FMUSIC_SetMasterVolume(module_, static_cast<int>(volume() * 256.0));
//    }
//};
//
Gosu::Song::Song(Audio& audio, const std::wstring& filename)
{
//    Buffer buf;
//	loadFile(buf, filename);
//	Type type = stStream;
//    
//	using boost::iends_with;
//	if (iends_with(filename, ".mod") || iends_with(filename, ".mid") ||
//	    iends_with(filename, ".s3m") || iends_with(filename, ".it") ||
//	    iends_with(filename, ".xm"))
//	{
//	      type = stModule;
//	}
//	
//    // Forward.
//	Song(audio, type, buf.frontReader()).data.swap(data);
}

Gosu::Song::Song(Audio& audio, Type type, Reader reader)
{
//    switch (type)
//    {
//    case stStream:
//        data.reset(new StreamData(reader));
//        break;
//
//    case stModule:
//        data.reset(new ModuleData(reader));
//        break;
//
//    default:
//        throw std::logic_error("Invalid song type");
//    }
}

Gosu::Song::~Song()
{
    if (alChannelManagement)
        stop();
}

Gosu::Song* Gosu::Song::currentSong()
{
    return curSong;
}

void Gosu::Song::play(bool looping)
{
//    if (curSong && curSong != this)
//    {
//        curSong->stop();
//        assert(curSong == 0);
//    }
//
//    data->play(looping);
//    curSong = this; // may be redundant
}

void Gosu::Song::pause()
{
//    if (curSong == this)
//        data->pause(); // may be redundant
}

bool Gosu::Song::paused() const
{
//    return curSong == this && data->paused();
return false;
}

void Gosu::Song::stop()
{
//    if (curSong == this)
//    {
//        data->stop();
//        curSong = 0;
//    }
}

bool Gosu::Song::playing() const
{
//    return curSong == this && !data->paused();
return false;
}

double Gosu::Song::volume() const
{
//    return data->volume();
return 0;
}

void Gosu::Song::changeVolume(double volume)
{
//    data->changeVolume(volume);
}
