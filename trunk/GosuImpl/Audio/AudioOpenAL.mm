#include <GosuImpl/MacUtility.hpp>
#include <GosuImpl/Audio/AudioToolboxFile.hpp>
#include <GosuImpl/Audio/ALChannelManagement.hpp>
#include <GosuImpl/Audio/OggFile.hpp>

#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

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
    
    bool isOggFile(Gosu::Reader reader)
    {
        char magicBytes[4];
        reader.read(magicBytes, 4);
        return magicBytes[0] == 'O' && magicBytes[1] == 'g' &&
               magicBytes[2] == 'g' && magicBytes[3] == 'S';
    }

    bool isOggFile(const std::wstring& filename)
    {
        Gosu::File file(filename);
        return isOggFile(file.frontReader());
    }
    
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

void Gosu::Audio::update()
{
    if (Song::currentSong())
        Song::currentSong()->update();
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return false;
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool Gosu::SampleInstance::paused() const
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return false;
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

void Gosu::SampleInstance::pause()
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourcePause(source);
}

void Gosu::SampleInstance::resume()
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED)
        alSourcePlay(source);
}

void Gosu::SampleInstance::stop()
{
    NSUInteger source = alChannelManagement->sourceIfStillPlaying(handle, extra);
    if (source == ALChannelManagement::NO_SOURCE)
        return;
    alSourceStop(source);
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
        
        if (!alChannelManagement)
            return;
            
        alDeleteBuffers(1, &buffer);
    }
};

Gosu::Sample::Sample(Audio& audio, const std::wstring& filename)
{
    if (isOggFile(filename))
    {
        Gosu::Buffer buffer;
        Gosu::loadFile(buffer, filename);
        OggFile oggFile(buffer.frontReader());
        data.reset(new SampleData(oggFile));
    }
    else
    {
        AudioToolboxFile audioFile(filename);
        data.reset(new SampleData(audioFile));
    }
}

Gosu::Sample::Sample(Audio& audio, Reader reader)
{
    if (isOggFile(reader))
    {
        OggFile oggFile(reader);
        data.reset(new SampleData(oggFile));
    }
    else
    {
        AudioToolboxFile audioFile(reader);
        data.reset(new SampleData(audioFile));
    }
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
    double volume_;

protected:
    BaseData() : volume_(1) {}
    virtual void applyVolume() = 0;

public:
    virtual ~BaseData() {}
    
    virtual void play(bool looping) = 0;
    virtual void pause() = 0;
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

class Gosu::Song::StreamData : public BaseData
{
    boost::scoped_ptr<AudioFile> file;
    NSUInteger buffers[2];
    
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
    
    bool streamToBuffer(NSUInteger buffer)
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
            file.reset(new AudioToolboxFile(filename));
        alGenBuffers(2, buffers);
    }

    StreamData(Reader reader)
    {
        if (isOggFile(reader))
            file.reset(new OggFile(reader));
        else
            file.reset(new AudioToolboxFile(reader));
        alGenBuffers(2, buffers);
    }
    
    ~StreamData()
    {
        stop();
        alDeleteBuffers(2, buffers);
    }
    
    void play(bool looping)
    {
        stop();
        file->rewind();
        
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
        {
            alSource3f(source, AL_POSITION, 0, 0, 0);
            alSourcef(source, AL_GAIN, volume());
            alSourcef(source, AL_PITCH, 1);
            alSourcei(source, AL_LOOPING, AL_FALSE); // need to implement this manually...

            streamToBuffer(buffers[0]);
            streamToBuffer(buffers[1]);

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
            int queued;
            alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
            NSUInteger buffer;
            while (queued--)
                alSourceUnqueueBuffers(source, 1, &buffer);
            
            int processed;
            alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
            while (processed--)
                alSourceUnqueueBuffers(source, 1, &buffer);
        }
    }
    
    void pause()
    {
        int source = lookupSource();
        if (source != ALChannelManagement::NO_SOURCE)
            alSourcePause(source);
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
        
        int processed;
        bool active = true;
        
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        
        for (int i = 0; i < processed && active; ++i)
        {
            NSUInteger buffer;
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
            // We got starved but there is nothing to play left, stop this
            stop();
            curSong = 0;
        }
    }
};

void Gosu::Song::update()
{
    data->update();
}

Gosu::Song::Song(Audio& audio, const std::wstring& filename)
: data(new StreamData(filename))
{
}

Gosu::Song::Song(Audio& audio, Type type, Reader reader)
: data(new StreamData(reader))
{
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
    if (curSong && curSong != this)
    {
        curSong->stop();
        assert(curSong == 0);
    }

    data->play(looping);
    curSong = this; // may be redundant
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
