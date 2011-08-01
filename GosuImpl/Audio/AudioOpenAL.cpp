#include <GosuImpl/Audio/ALChannelManagement.hpp>
#include <GosuImpl/Audio/OggFile.hpp>

#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>

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
#include <GosuImpl/Audio/AudioToolboxFile.hpp>
#define WAVE_FILE AudioToolboxFile
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <GosuImpl/Audio/SndFile.hpp>
#define WAVE_FILE SndFile
#endif

#ifdef GOSU_IS_IPHONE
#import <AVFoundation/AVFoundation.h>
#endif

using namespace std;

namespace
{
    using namespace Gosu;
    
    bool isOggFile(Gosu::Reader reader)
    {
        char magicBytes[4];
        reader.read(magicBytes, 4);
        return magicBytes[0] == 'O' && magicBytes[1] == 'g' &&
               magicBytes[2] == 'g' && magicBytes[3] == 'S';
    }

    bool isOggFile(const wstring& filename)
    {
        Gosu::File file(filename);
        return isOggFile(file.frontReader());
    }
    
    Song* curSong = 0;
    bool curSongLooping;
}

// TODO: What is the NSAutoreleasePool good for?
#ifdef GOSU_IS_MAC
#include <GosuImpl/MacUtility.hpp>
    #define CONSTRUCTOR_COMMON \
        ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]); \
        if (!alChannelManagement.get()) \
            alChannelManagement.reset(new ALChannelManagement)
#else
    #define CONSTRUCTOR_COMMON \
        if (!alChannelManagement.get()) \
            alChannelManagement.reset(new ALChannelManagement)
#endif    

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return false;
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool Gosu::SampleInstance::paused() const
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return false;
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

void Gosu::SampleInstance::pause()
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourcePause(source);
}

void Gosu::SampleInstance::resume()
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED)
        alSourcePlay(source);
}

void Gosu::SampleInstance::stop()
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourceStop(source);
}

void Gosu::SampleInstance::changeVolume(double volume)
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourcef(source, AL_GAIN, volume);
}

void Gosu::SampleInstance::changePan(double pan)
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    // TODO: This is not the old panning behavior!
    alSource3f(source, AL_POSITION, pan * 10, 0, 0);
}

void Gosu::SampleInstance::changeSpeed(double speed)
{
    ALuint source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourcef(source, AL_PITCH, speed);
}

struct Gosu::Sample::SampleData
{
    ALuint buffer, source;

    SampleData(AudioFile& audioFile)
    {
        alGenBuffers(1, &buffer);
        alBufferData(buffer,
                     audioFile.format(),
                     &audioFile.decodedData().front(),
                     audioFile.decodedData().size(),
                     audioFile.sampleRate());
    }
    
    ~SampleData()
    {
        // It's hard to free things in the right order in Ruby/Gosu.
        // Make sure buffer isn't deleted after the context/device are shut down.
        if (!alChannelManagement.get())
            return;
            
        alDeleteBuffers(1, &buffer);
    }

private:
    SampleData(const SampleData&);
    SampleData& operator=(const SampleData&);
};

Gosu::Sample::Sample(const std::wstring& filename)
{
    CONSTRUCTOR_COMMON;

    if (isOggFile(filename))
    {
        Gosu::Buffer buffer;
        Gosu::loadFile(buffer, filename);
        OggFile oggFile(buffer.frontReader());
        data.reset(new SampleData(oggFile));
    }
    else
    {
        WAVE_FILE audioFile(filename);
        data.reset(new SampleData(audioFile));
    }
}

Gosu::Sample::Sample(Reader reader)
{
    CONSTRUCTOR_COMMON;

    if (isOggFile(reader))
    {
        OggFile oggFile(reader);
        data.reset(new SampleData(oggFile));
    }
    else
    {
        WAVE_FILE audioFile(reader);
        data.reset(new SampleData(audioFile));
    }
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
        
    ALuint source = alChannelManagement->sourceIfStillPlaying(channelAndToken.first,
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

class Gosu::Song::BaseData
{
    BaseData(const BaseData&);
    BaseData& operator=(const BaseData&);
    
    double volume_;

protected:
    BaseData() : volume_(1) {}
    virtual void applyVolume() = 0;

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
    
    void changeVolume(double volume)
    {
        volume_ = clamp(volume, 0.0, 1.0);
        applyVolume();
    }
};

#ifdef GOSU_IS_IPHONE
// AVAudioPlayer impl
class Gosu::Song::ModuleData : public BaseData
{
    ObjRef<AVAudioPlayer> player;
    
    void applyVolume()
    {
        player.obj().volume = volume();
    }
    
public:
    ModuleData(const std::wstring& filename)
    {
        std::string utf8Filename = Gosu::wstringToUTF8(filename);
        ObjRef<NSString> nsFilename([[NSString alloc] initWithUTF8String: utf8Filename.c_str()]);
        ObjRef<NSURL> url([[NSURL alloc] initFileURLWithPath: nsFilename.obj()]);
        player.reset([[AVAudioPlayer alloc] initWithContentsOfURL: url.obj() error: NULL]);
    }
    
    void play(bool looping)
    {
        if (paused())
            stop();
        player.obj().numberOfLoops = looping ? -1 : 0;
        [player.obj() play];
    }
    
    void pause()
    {
        [player.obj() pause];
    }
    
    void resume()
    {
        [player.obj() play];
    }
    
    bool paused() const
    {
        return !player.obj().playing;
    };
    
    void stop()
    {
        [player.obj() stop];
        player.obj().currentTime = 0;
    }
    
    void update()
    {
    }
};
#endif

// AudioFile impl
class Gosu::Song::StreamData : public BaseData
{
    std::auto_ptr<AudioFile> file;
    ALuint buffers[2];
    
    void applyVolume()
    {
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
            alSourcef(source, AL_GAIN, volume());
    }
    
    int lookupSource() const
    {
        return alChannelManagement->sourceForSongs();
    }
    
    bool streamToBuffer(ALuint buffer)
    {
        #ifdef GOSU_IS_IPHONE
        static const unsigned BUFFER_SIZE = 4096 * 4;
        #else
        static const unsigned BUFFER_SIZE = 4096 * 8;
        #endif
        char audioData[BUFFER_SIZE];
        std::size_t readBytes = file->readData(audioData, BUFFER_SIZE);
        if (readBytes > 0)
            alBufferData(buffer, file->format(), audioData, readBytes, file->sampleRate());
        return readBytes > 0;
    }
    
public:
    StreamData(const std::wstring& filename)
    {
        if (isOggFile(filename))
        {
            Gosu::File sourceFile(filename);
            file.reset(new OggFile(sourceFile.frontReader()));
        }
        else
            file.reset(new WAVE_FILE(filename));
        alGenBuffers(2, buffers);
    }

    StreamData(Reader reader)
    {
        if (isOggFile(reader))
            file.reset(new OggFile(reader));
        else
            file.reset(new WAVE_FILE(reader));
        alGenBuffers(2, buffers);
    }
    
    ~StreamData()
    {
        if (alChannelManagement.get())
        {
            stop();
            alDeleteBuffers(2, buffers);
        }
    }
    
    void play(bool looping)
    {
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
        {
            alSource3f(source, AL_POSITION, 0, 0, 0);
            alSourcef(source, AL_GAIN, volume());
            alSourcef(source, AL_PITCH, 1);
            alSourcei(source, AL_LOOPING, AL_FALSE); // need to implement this manually...

            streamToBuffer(buffers[0]);
            streamToBuffer(buffers[1]);
            
            // TODO: Not good for songs with less than two buffers full of data.
            
            alSourceQueueBuffers(source, 2, buffers);
            alSourcePlay(source);
        }
    }

    void stop()
    {
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
        {
            alSourceStop(source);

            ALuint buffer;
            
            // The number of QUEUED buffers apparently includes the number of
            // PROCESSED ones, so getting rid of the QUEUED ones is enough.
            
            int queued;
            alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
            while (queued--)
                alSourceUnqueueBuffers(source, 1, &buffer);

            //int processed;
            //alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
            //while (processed--)
            //    alSourceUnqueueBuffers(source, 1, &buffer);
        }
        file->rewind();
    }
    
    void pause()
    {
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
            alSourcePause(source);
    }
    
    void resume()
    {
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
            alSourcePlay(source);
    }
    
    bool paused() const
    {
        int source = lookupSource();
        if (source == ALChannelManagement::NO_SOURCE)
            return false;
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        return state == AL_PAUSED;
    }
    
    void update()
    {
        int source = lookupSource();

        ALuint buffer;
        int processed;
        bool active = true;
        
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        for (int i = 0; i < processed; ++i)
        {
            alSourceUnqueueBuffers(source, 1, &buffer);
            active = streamToBuffer(buffer);
            if (active)
                alSourceQueueBuffers(source, 1, &buffer);
        }
        
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (active && state != AL_PLAYING && state != AL_PAUSED)
        {
            // We seemingly got starved.
            alSourcePlay(source);
        }
        else if (!active)
        {
            // We got starved and there is nothing to play left.
            // In any case, shut down the playback logic for a moment.
            stop();

            if (curSongLooping)
                // Start anew.
                play(true);
            else
                // Let the world know we're finished.
                curSong = 0;
        }
    }
};

// TODO: Move into proper internal header
namespace Gosu { bool isExtension(const wchar_t* str, const wchar_t* ext); }

Gosu::Song::Song(const std::wstring& filename)
{
#ifdef GOSU_IS_IPHONE
    if (isExtension(filename.c_str(), L".mp3") ||
        isExtension(filename.c_str(), L".aac") ||
        isExtension(filename.c_str(), L".m4a"))
        data.reset(new ModuleData(filename));
    else
#endif
    {
        CONSTRUCTOR_COMMON;
        data.reset(new StreamData(filename));
    }
}

Gosu::Song::Song(Reader reader)
{
    CONSTRUCTOR_COMMON;

    data.reset(new StreamData(reader));
}

Gosu::Song::~Song()
{
    stop();
}

Gosu::Song* Gosu::Song::currentSong()
{
    return curSong;
}

void Gosu::Song::play(bool looping)
{
    if (paused())
        data->resume();
    
    if (curSong && curSong != this)
    {
        curSong->stop();
        assert(curSong == 0);
    }
    
    if (curSong == 0)
        data->play(looping);
    
    curSong = this;
    curSongLooping = looping;
}

void Gosu::Song::pause()
{
    if (curSong == this)
        data->pause(); // may be redundant
}

bool Gosu::Song::paused() const
{
    return curSong == this && data->paused();
}

void Gosu::Song::stop()
{
    if (curSong == this)
    {
        data->stop();
        curSong = 0;
    }
}

bool Gosu::Song::playing() const
{
    return curSong == this && !data->paused();
}

double Gosu::Song::volume() const
{
    return data->volume();
}

void Gosu::Song::changeVolume(double volume)
{
    data->changeVolume(volume);
}

void Gosu::Song::update()
{
    if (currentSong())
        currentSong()->data->update();
}

// Deprecated constructors.

Gosu::Sample::Sample(Audio& audio, const std::wstring& filename)
{
    Sample(filename).data.swap(data);
}

Gosu::Sample::Sample(Audio& audio, Reader reader)
{
    Sample(reader).data.swap(data);
}

Gosu::Song::Song(Audio& audio, const std::wstring& filename)
{
    data = Song(filename).data;
}

Gosu::Song::Song(Audio& audio, Type type, Reader reader)
{
    data = Song(reader).data;
}
