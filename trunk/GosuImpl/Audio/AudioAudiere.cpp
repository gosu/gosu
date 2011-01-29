#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <audiere.h>
using namespace audiere;

namespace Gosu
{
    namespace
    {
        Song* curSong = 0;
        
        // Gosu::Buffer-based implementation of Audiere's File interface.
        // Provided since Audiere uses stdio to open files, which may not support Unicode
        // filenames outside of the current codepage on Windows(?).
        class MemoryBuffer : boost::noncopyable
        {
            Gosu::Buffer buffer;
            FilePtr file;

        public:
            MemoryBuffer(Gosu::Reader reader)
            {
                buffer.resize(reader.resource().size() - reader.position());
                reader.read(buffer.data(), buffer.size());
                file = audiere::CreateMemoryFile(buffer.data(), buffer.size());
            }

            MemoryBuffer(const std::wstring& filename)
            {
                Gosu::loadFile(buffer, filename);
                file = audiere::CreateMemoryFile(buffer.data(), buffer.size());
            }

            operator const FilePtr&()
            {
                return file;
            }
        };

        class StreamRegistry : public RefImplementation<StopCallback>, boost::noncopyable
        {
            struct NumberedStream
            {
                int extra;
                OutputStreamPtr stream;
            };
            std::vector<NumberedStream> streams;
            
            ADR_METHOD(void) streamStopped(StopEvent* event)
            {
                if (event->getReason() == StopEvent::STREAM_ENDED)
                    if (!clear(event->getOutputStream()) && curSong)
                        curSong = 0;
            }

        public:
            StreamRegistry()
            {
                streams.reserve(100);
            }
            
            OutputStreamPtr get(int handle, int extra) const
            {
                if (handle < streams.size() && streams[handle].extra == extra)
                    return streams[handle].stream;
                return OutputStreamPtr();
            }
            
            void put(OutputStreamPtr stream, int& handle, int& extra)
            {
                handle = 0;
                for (handle; handle < streams.size(); ++handle)
                    if (!streams[handle].stream)
                    {
                        streams[handle].stream = stream;
                        extra = ++streams[handle].extra;
                        return;
                    }
                // handle == streams.size() <3  
                NumberedStream newStream = { extra = 0, stream };
                streams.push_back(newStream);
            }
            
            void clear(int handle)
            {
                streams.at(handle).stream = 0;
            }

            bool clear(OutputStreamPtr stream)
            {
                BOOST_FOREACH (NumberedStream& numberedStream, streams)
                    if (numberedStream.stream == stream)
                    {
                        numberedStream.stream = 0;
                        return true;
                    }
                return false;
            }
        };
        
        // Intentionally leak. Let Windows clean up on program exit.
        StreamRegistry& streams = *(new StreamRegistry);

        // Since audiere::OpenDevice is (contains) the first call to audiere.dll, we wrap
        // the call in error handling code to see if the lazily linked DLL is there.
        bool getDevice(AudioDevice*& device)
        {
            #ifdef GOSU_IS_WIN
            // Copied and pasted from MSDN.
            #define FACILITY_VISUALCPP  ((LONG)0x6d)
            #define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)
            #define BAD_MOD VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)
            
            __try
            {
            #endif
                return (device = OpenDevice());
            #ifdef GOSU_IS_WIN
            }
            __except ((GetExceptionCode() == BAD_MOD) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                return false;
            }
            #undef BAD_MOD
            #undef VcppException
            #undef FACILITY_VISUALCPP
            #endif
        }

        AudioDevice* device()
        {
            static AudioDevice* device = 0;
            if (device == 0)
            {
                if (!getDevice(device))
                    throw std::runtime_error("Could not initialize audiere or library not found");

                device->registerCallback(&streams);

                // Never free. Especially important in Ruby version, where GC order is undefined
                device->ref();
            }
            return device;
        }
        
    }
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    OutputStreamPtr stream = streams.get(handle, extra);
    return stream && stream->isPlaying();
}

bool Gosu::SampleInstance::paused() const
{
    OutputStreamPtr stream = streams.get(handle, extra);
    return stream && !stream->isPlaying();
}

void Gosu::SampleInstance::pause()
{
    if (OutputStreamPtr stream = streams.get(handle, extra))
        stream->stop();
}

void Gosu::SampleInstance::resume()
{
    if (OutputStreamPtr stream = streams.get(handle, extra))
        stream->play();
}

void Gosu::SampleInstance::stop()
{
    if (OutputStreamPtr stream = streams.get(handle, extra))
        stream->stop(), streams.clear(handle);
}

void Gosu::SampleInstance::changeVolume(double volume)
{
    if (OutputStreamPtr stream = streams.get(handle, extra))
        stream->setVolume(volume);
}

void Gosu::SampleInstance::changePan(double pan)
{
    if (OutputStreamPtr stream = streams.get(handle, extra))
        stream->setPan(clamp<float>(pan, -1.f, +1.f));
}

void Gosu::SampleInstance::changeSpeed(double speed)
{
    if (OutputStreamPtr stream = streams.get(handle, extra))
        stream->setPitchShift(clamp<float>(speed, 0.5f, 2.0f));
}

struct Gosu::Sample::SampleData : boost::noncopyable
{
    SampleBufferPtr buffer;
};

Gosu::Sample::Sample(const std::wstring& filename)
:   data(new SampleData)
{
    device(); // Implicitly open audio device
    
    SampleSourcePtr source = OpenSampleSource(MemoryBuffer(filename), FF_AUTODETECT);
    data->buffer = CreateSampleBuffer(source);
}

Gosu::Sample::Sample(Reader reader)
{
    device(); // Implicitly open audio device
    
    SampleSourcePtr source = audiere::OpenSampleSource(MemoryBuffer(reader), FF_AUTODETECT);
    data->buffer = CreateSampleBuffer(source);
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed,
    bool looping) const
{
    OutputStreamPtr stream = device()->openStream(data->buffer->openStream());
    int handle, extra;
    streams.put(stream, handle, extra);

    SampleInstance instance(handle, extra);
    instance.changeVolume(volume);
    instance.changeSpeed(speed);
    stream->setRepeat(looping);
    stream->play();
    return instance;
}

Gosu::SampleInstance Gosu::Sample::playPan(double pan, double volume,
    double speed, bool looping) const
{
    OutputStreamPtr stream = device()->openStream(data->buffer->openStream());
    int handle, extra;
    streams.put(stream, handle, extra);

    SampleInstance instance(handle, extra);
    instance.changePan(pan);
    instance.changeVolume(volume);
    instance.changeSpeed(speed);
    stream->setRepeat(looping);
    stream->play();
    return instance;
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
    virtual bool playing() const = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    
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
    MemoryBuffer buffer;
    OutputStreamPtr stream;
    SampleInstance instance;
    bool loop;

    void init()
    {
        int handle, extra;
        stream = OpenSound(device(), buffer, true);
        streams.put(stream, handle, extra);
        instance = SampleInstance(handle, extra);
        loop = false;
    }

public:
    StreamData(Gosu::Reader reader)
    :   buffer(reader), instance(-1, -1)
    {
        init();
    }

    StreamData(const std::wstring& filename)
    :   buffer(filename), instance(-1, -1)
    {
        init();
    }
    
    void play(bool looping)
    {
        applyVolume();
        if (looping != loop)
            stream->setRepeat(loop = looping);
        stream->play();
    }

    bool playing() const
    {
        return instance.playing();
    }
    
    void pause()
    {
        instance.pause();
    }
    
    void stop()
    {
        instance.pause();
        stream->reset();
    }
    
    void applyVolume()
    {
        instance.changeVolume(volume());
    }
};

Gosu::Song::Song(const std::wstring& filename)
: data(new StreamData(filename))
{
}

Gosu::Song::Song(Reader reader)
: data(new StreamData(reader))
{
}

Gosu::Song::~Song()
{
}

Gosu::Song* Gosu::Song::currentSong()
{
    if (curSong)
        return curSong;
    return 0;
}

void Gosu::Song::play(bool looping)
{
    if (Gosu::Song* cur = currentSong())
        if (cur != this)
        {
            cur->stop();
            assert (currentSong() == 0);
        }

    data->play(looping); // May be redundant
    curSong = this;      // May be redundant
}

void Gosu::Song::pause()
{
    if (curSong == this && data->playing())
        data->pause();
}

bool Gosu::Song::paused() const
{
    return curSong == this && !data->playing();
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
    assert (!(curSong != this && data->playing()));
    return data->playing();
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
    device()->update();
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
    Song(filename).data.swap(data);
}

Gosu::Song::Song(Audio& audio, Type type, Reader reader)
{
    Song(reader).data.swap(data);
}
