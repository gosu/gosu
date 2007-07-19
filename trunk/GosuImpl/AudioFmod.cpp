#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <fmod.h>
#include <fmod_errors.h>

namespace Gosu
{
    namespace
    {
        GOSU_NORETURN void throwLastFMODError()
        {
            throw std::runtime_error(::FMOD_ErrorString(::FSOUND_GetError()));
        }

        inline unsigned char fmodCheck(unsigned char retVal)
        {
            if (retVal == 0 && ::FSOUND_GetError() != FMOD_ERR_NONE)
                throwLastFMODError();

            return retVal;
        }

        bool fmodInitialized = false;
        Song* curSong = 0;

        #ifdef GOSU_IS_WIN
        bool setWindow(HWND window)
        {
            // Copied and pasted from MSDN.
            #define FACILITY_VISUALCPP  ((LONG)0x6d)
            #define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)
            #define BAD_MOD VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)

            __try
            {
                ::FSOUND_SetHWND(reinterpret_cast<void*>(window));
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
        #endif
    }
}

#ifdef GOSU_IS_WIN
Gosu::Audio::Audio(HWND window)
{
    if (!setWindow(window))
        throw std::runtime_error("Could not load fmod.dll");
#else
Gosu::Audio::Audio()
{
#endif

    if (fmodInitialized)
        throw std::logic_error("Multiple Gosu::Audio instances not supported");

    fmodCheck(::FSOUND_Init(44100, 32, 0));
    fmodInitialized = true;
}

Gosu::Audio::~Audio()
{
    assert(fmodInitialized);

    ::FSOUND_Close();
    fmodInitialized = false;
}

struct Gosu::Sample::SampleData : boost::noncopyable
{
    FSOUND_SAMPLE* rep;

    SampleData()
    : rep(0)
    {
    }

    ~SampleData()
    {
        if (rep != 0)
            ::FSOUND_Sample_Free(rep);
    }
};

Gosu::Sample::Sample(Audio& audio, const std::wstring& filename)
{
	Buffer buf;
	loadFile(buf, filename);

	// Forward.
	Sample(audio, buf.frontReader()).data.swap(data);
}

Gosu::Sample::Sample(Audio& audio, Reader reader)
{
    std::vector<char> buffer(reader.resource().size() - reader.position());
    reader.read(&buffer.front(), buffer.size());

    data.reset(new SampleData);
    data->rep = ::FSOUND_Sample_Load(FSOUND_FREE | FSOUND_UNMANAGED,
        &buffer.front(), FSOUND_NORMAL | FSOUND_LOADMEMORY, 0, buffer.size());
    if (data->rep == 0)
        throwLastFMODError();
}

Gosu::Sample::~Sample()
{
    assert(fmodInitialized);
}

void Gosu::Sample::play(double volume, double speed) const
{
    int channel = ::FSOUND_PlaySound(FSOUND_FREE, data->rep);

    if (channel > 0)
    {
        ::FSOUND_SetPan(channel, FSOUND_STEREOPAN);

        if (volume != 1)
        {
            double vol = boundBy(volume * 255, 0.0, 255.0);

            ::FSOUND_SetVolume(channel, trunc(vol));
        }

        if (speed != 1)
        {
            double freq = boundBy(speed * ::FSOUND_GetFrequency(channel),
                    100.0, 705600.0);

            ::FSOUND_SetFrequency(channel, trunc(freq));
        }
    }
}

void Gosu::Sample::playPan(double pan, double volume, double speed) const
{
    int channel = ::FSOUND_PlaySound(FSOUND_FREE, data->rep);

    if (channel > 0)
    {
        ::FSOUND_SetPan(channel, trunc(boundBy(pan * 127 + 127, 0.0, 255.0)));

        if (volume != 1)
        {
            double vol = boundBy(volume * 255, 0.0, 255.0);

            ::FSOUND_SetVolume(channel, trunc(vol));
        }

        if (speed != 1)
        {
            double freq = boundBy(speed * ::FSOUND_GetFrequency(channel),
                    100.0, 705600.0);

            ::FSOUND_SetFrequency(channel, trunc(freq));
        }
    }
}

class Gosu::Song::BaseData : boost::noncopyable
{
public:
    virtual ~BaseData() {}

    virtual void play() = 0;
    virtual void stop() = 0;
};

class Gosu::Song::StreamData : public BaseData
{
    FSOUND_STREAM* stream;
    std::vector<char> buffer;

    static signed char F_CALLBACKAPI endSongCallback(FSOUND_STREAM*, void*,
        int, void*)
    {
        curSong = 0;
        return 0;
    }

public:
    StreamData(Gosu::Reader reader)
    : stream(0)
    {
        buffer.resize(reader.resource().size() - reader.position());
        reader.read(&buffer[0], buffer.size());

#ifndef GOSU_ALLOW_MP3
        if (buffer.size() > 2 &&
            ((buffer[0] == '\xff' && (buffer[1] & 0xfe) == '\xfa') ||
            (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3')))
        {
            throw std::runtime_error("MP3 file playback not allowed");
        }
#endif

        stream = ::FSOUND_Stream_Open(&buffer[0], FSOUND_LOADMEMORY, 0,
            buffer.size());
        if (stream == 0)
            throwLastFMODError();
        
        ::FSOUND_Stream_SetEndCallback(stream, endSongCallback, 0);
    }

    ~StreamData()
    {
        if (stream != 0)
            ::FSOUND_Stream_Close(stream);
    }

    void play()
    {
        if (::FSOUND_Stream_Play(FSOUND_FREE, stream) == -1)
            throwLastFMODError();
    }

    void stop()
    {
        fmodCheck(::FSOUND_Stream_Stop(stream));
    }
};

class Gosu::Song::ModuleData : public Gosu::Song::BaseData
{
    FMUSIC_MODULE* module_;

public:
    ModuleData(Reader reader)
    : module_(0)
    {
        std::vector<char> buffer(reader.resource().size() - reader.position());
        reader.read(&buffer[0], buffer.size());

        module_ = ::FMUSIC_LoadSongEx(&buffer[0], 0, buffer.size(),
            FSOUND_LOADMEMORY | FSOUND_LOOP_OFF, 0, 0);
        if (module_ == 0)
            throwLastFMODError();
    }

    ~ModuleData()
    {
        if (module_ != 0)
            ::FMUSIC_FreeSong(module_);
    }

    void play()
    {
        fmodCheck(::FMUSIC_PlaySong(module_));
    }

    void stop()
    {
        fmodCheck(::FMUSIC_StopSong(module_));
    }
};

Gosu::Song::Song(Audio& audio, const std::wstring& filename)
{
    Buffer buf;
	  loadFile(buf, filename);
	  Type type = stStream;

	  using boost::iends_with;
	  if (iends_with(filename, ".mod") || iends_with(filename, ".mid") ||
		    iends_with(filename, ".s3m") || iends_with(filename, ".it") ||
		    iends_with(filename, ".xm"))
	  {
	      type = stModule;
	  }
	
    // Forward.
	  Song(audio, type, buf.frontReader()).data.swap(data);
}

Gosu::Song::Song(Audio& audio, Type type, Reader reader)
{
    switch (type)
    {
    case stStream:
        data.reset(new StreamData(reader));
        break;

    case stModule:
        data.reset(new ModuleData(reader));
        break;

    default:
        throw std::logic_error("Invalid song type");
    }
}

Gosu::Song::~Song()
{
    assert(fmodInitialized);
    stop();
}

void Gosu::Song::play()
{
    if (curSong)
        curSong->stop();

    assert(curSong == 0);

    data->play();
    curSong = this;
}

void Gosu::Song::stop()
{
    if (playing())
    {
        data->stop();
        curSong = 0;
    }
}

bool Gosu::Song::playing() const
{
    return curSong == this;
}
