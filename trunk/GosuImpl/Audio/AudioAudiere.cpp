#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
//#include <Gosu/IO.hpp>
//#include <Gosu/Utility.hpp>
//#include <boost/algorithm/string.hpp>
//#include <cassert>
//#include <cstdlib>
#include <algorithm>
//#include <stdexcept>
//#include <vector>

#include <audiere.h>
using namespace audiere;

namespace Gosu
{
    namespace
    {
        Song* curSong = 0;
        
        // Since audiere::OpenDevice is (contains) the first call to audiere.dll, we wrap
        // the call in error handling code to see if the lazily linked DLL is there.
        bool getDevice(AudioDevice*& device)
        {
            // Copied and pasted from MSDN.
            #define FACILITY_VISUALCPP  ((LONG)0x6d)
            #define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)
            #define BAD_MOD VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)

            __try
            {
                device = OpenDevice();
            }
            __except ((GetExceptionCode() == BAD_MOD) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                return false;
            }
            return true;

            #undef BAD_MOD
            #undef VcppException
            #undef FACILITY_VISUALCPP
        }

        AudioDevice* device()
        {
            static AudioDevice* device = 0;
            if (device == 0)
            {
                if (!getDevice(device))
                    throw std::runtime_error("Could not find audiere.dll");
                device->ref(); // Never free. Especially important in Ruby version, where GC order is undefined
            }
            return device;
        }
        
        // Gosu::Buffer-based implementation of Audiere's File interface.
        // Provided since Audiere uses stdio to open files, which may not support Unicode
        // outside of the current codepage on Windows(?).
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

        class StreamRegistry
        {
            struct NumberedStream
            {
                int extra;
                OutputStreamPtr stream;
            };
            mutable std::vector<NumberedStream> streams;

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
        };
        
        StreamRegistry streams;
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
    virtual void pause() = 0;
    virtual bool paused() const = 0;
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
    //FSOUND_STREAM* stream;
    //int handle;
    //std::vector<char> buffer;

    //static signed char F_CALLBACKAPI endSongCallback(FSOUND_STREAM*, void*,
    //    int, void* self)
    //{
    //    curSong = 0;
    //    static_cast<StreamData*>(self)->handle = -1;
    //    return 0;
    //}

public:
    StreamData(Gosu::Reader reader)
    //: stream(0), handle(-1)
    {
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
    }
    
    ~StreamData()
    {
        //if (stream != 0)
        //    FSOUND_Stream_Close(stream);
    }
    
    void play(bool looping)
    {
        //if (handle == -1)
        //{
        //    handle = FSOUND_Stream_Play(FSOUND_FREE, stream);
        //        FSOUND_Stream_SetLoopCount(stream, looping ? -1 : 0);
        //}
        //else if (paused())
        //    FSOUND_SetPaused(handle, 0);
        //applyVolume();
    }
    
    void pause()
    {
        //if (handle != -1)
        //    FSOUND_SetPaused(handle, 1);
    }
    
    bool paused() const
    {
        return false;
        //return handle != -1 && FSOUND_GetPaused(handle);
    }
    
    void stop()
    {
        //fmodCheck(FSOUND_Stream_Stop(stream));
        //handle = -1; // The end callback is NOT being called!
    }
    
    void applyVolume()
    {
        //if (handle != -1)
            //FSOUND_SetVolume(handle, static_cast<int>(volume() * 255));
    }
};

/*class Gosu::Song::ModuleData : public Gosu::Song::BaseData
{
    FMUSIC_MODULE* module_;

public:
    ModuleData(Reader reader)
    : module_(0)
    {
        std::vector<char> buffer(reader.resource().size() - reader.position());
        reader.read(&buffer[0], buffer.size());

        module_ = FMUSIC_LoadSongEx(&buffer[0], 0, buffer.size(),
            FSOUND_LOADMEMORY | FSOUND_LOOP_OFF, 0, 0);
        if (module_ == 0)
            throwLastFMODError();
    }

    ~ModuleData()
    {
        if (module_ != 0)
            FMUSIC_FreeSong(module_);
    }

    void play(bool looping)
    {
        if (paused())
            FMUSIC_SetPaused(module_, 0);
        else
            FMUSIC_PlaySong(module_);
        FMUSIC_SetLooping(module_, looping);
        applyVolume();
    }
    
    void pause()
    {
        FMUSIC_SetPaused(module_, 1);
    }
    
    bool paused() const
    {
        return FMUSIC_GetPaused(module_);
    }

    void stop()
    {
        fmodCheck(FMUSIC_StopSong(module_));
        FMUSIC_SetPaused(module_, false);
    }
    
    void applyVolume()
    {
        // Weird as it may seem, the FMOD doc really says volume can
        // be 0 to 256, *inclusive*, for this function.
        FMUSIC_SetMasterVolume(module_, static_cast<int>(volume() * 256.0));
    }
};*/

Gosu::Song::Song(const std::wstring& filename)
{
    device(); // Implicitly open audio device
  
 //   Buffer buf;
	//loadFile(buf, filename);
	//Type type = stStream;
 //   
	//using boost::iends_with;
	//if (iends_with(filename, ".mod") || iends_with(filename, ".mid") ||
	//    iends_with(filename, ".s3m") || iends_with(filename, ".it") ||
	//    iends_with(filename, ".xm"))
	//{
	//      type = stModule;
	//}
	//
 //   // Forward.
	//Song(type, buf.frontReader()).data.swap(data);
}

Gosu::Song::Song(Type, Reader reader)
{
    device(); // Implicitly open audio device
    //
    //switch (type)
    //{
    //case stStream:
    //    data.reset(new StreamData(reader));
    //    break;

    //case stModule:
    //    data.reset(new ModuleData(reader));
    //    break;

    //default:
    //    throw std::logic_error("Invalid song type");
    //}
}

Gosu::Song::~Song()
{
}

Gosu::Song* Gosu::Song::currentSong()
{
    return curSong;
}

void Gosu::Song::play(bool looping)
{
    //if (curSong && curSong != this)
    //{
    //    curSong->stop();
    //    assert(curSong == 0);
    //}

    //data->play(looping);
    //curSong = this; // may be redundant
}

void Gosu::Song::pause()
{
    //if (curSong == this)
    //    data->pause(); // may be redundant
}

bool Gosu::Song::paused() const
{
    return false;//return curSong == this && data->paused();
}

void Gosu::Song::stop()
{
    //if (curSong == this)
    //{
    //    data->stop();
    //    curSong = 0;
    //}
}

bool Gosu::Song::playing() const
{
    return curSong == this;
    //return curSong == this && !data->paused();
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
    Song(type, reader).data.swap(data);
}
