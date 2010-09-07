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
    extern HWND __Gosu_HWND_for_FMOD;

    namespace
    {
        GOSU_NORETURN void throwLastFMODError()
        {
            throw std::runtime_error(FMOD_ErrorString(FSOUND_GetError()));
        }

        inline unsigned char fmodCheck(unsigned char retVal)
        {
            if (retVal == 0 && FSOUND_GetError() != FMOD_ERR_NONE)
                throwLastFMODError();

            return retVal;
        }

        #ifdef GOSU_IS_WIN
        bool setWindow()
        {
            // Copied and pasted from MSDN.
            #define FACILITY_VISUALCPP  ((LONG)0x6d)
            #define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)
            #define BAD_MOD VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)

            HWND fmodWindow = __Gosu_HWND_for_FMOD;
            if (fmodWindow == 0)
                fmodWindow = GetDesktopWindow();

            // Try to call the first function from fmod.dll, which is lazily
            // linked. With this guard, we can raise a catchable C++ exception
            // if the library is not found, instead of crashing the game.
            __try
            {
                FSOUND_SetHWND(fmodWindow);
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
        
        void requireFMOD()
        {
            static bool initialized = false;
            if (initialized)
                return;
                
            #ifdef GOSU_IS_WIN
            if (!setWindow())
                throw std::runtime_error("Could not load fmod.dll");
            #endif
            fmodCheck(FSOUND_Init(44100, 32, 0));
            
            initialized = true;
        }
        Song* curSong = 0;
    }
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    return FSOUND_IsPlaying(handle);
}

bool Gosu::SampleInstance::paused() const
{
    return FSOUND_GetPaused(handle);
}

void Gosu::SampleInstance::pause()
{
    FSOUND_SetPaused(handle, 1);
}

void Gosu::SampleInstance::resume()
{
    FSOUND_SetPaused(handle, 0);
}

void Gosu::SampleInstance::stop()
{
    FSOUND_StopSound(handle);
}

void Gosu::SampleInstance::changeVolume(double volume)
{
    FSOUND_SetVolume(handle, clamp<int>(volume * 255, 0, 255));
}

void Gosu::SampleInstance::changePan(double pan)
{
    FSOUND_SetPan(handle, clamp<int>(pan * 127 + 127, 0, 255));
}

void Gosu::SampleInstance::changeSpeed(double speed)
{
    FSOUND_SetFrequency(handle, clamp<int>(speed * extra, 100, 705600));
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
            FSOUND_Sample_Free(rep);
    }
};

Gosu::Sample::Sample(const std::wstring& filename)
{
    requireFMOD();
    
	Buffer buf;
	loadFile(buf, filename);

	// Forward.
	Sample(buf.frontReader()).data.swap(data);
}

Gosu::Sample::Sample(Reader reader)
{
    requireFMOD();
    
    std::vector<char> buffer(reader.resource().size() - reader.position());
    reader.read(&buffer.front(), buffer.size());

    data.reset(new SampleData);
    data->rep = FSOUND_Sample_Load(FSOUND_FREE | FSOUND_UNMANAGED,
        &buffer.front(), FSOUND_NORMAL | FSOUND_LOADMEMORY, 0, buffer.size());
    if (data->rep == 0)
        throwLastFMODError();
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed,
    bool looping) const
{
    int handle = FSOUND_PlaySound(FSOUND_FREE, data->rep);
    int freq;
    if (handle > 0)
    {
        if (looping)
            FSOUND_SetLoopMode(handle, FSOUND_LOOP_NORMAL);
        freq = FSOUND_GetFrequency(handle);
        FSOUND_SetPan(handle, FSOUND_STEREOPAN);
    }
    
    SampleInstance result(handle, freq);
    result.changeVolume(volume);
    result.changeSpeed(speed);
    return result;
}

Gosu::SampleInstance Gosu::Sample::playPan(double pan, double volume,
    double speed, bool looping) const
{
    int handle = FSOUND_PlaySound(FSOUND_FREE, data->rep);
    int freq;
    if (handle > 0)
    {
        if (looping)
            FSOUND_SetLoopMode(handle, FSOUND_LOOP_NORMAL);
        freq = FSOUND_GetFrequency(handle);
    }
    
    SampleInstance result(handle, freq);
    result.changeVolume(volume);
    result.changePan(pan);
    result.changeSpeed(speed);
    return result;
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
    FSOUND_STREAM* stream;
    int handle;
    std::vector<char> buffer;

    static signed char F_CALLBACKAPI endSongCallback(FSOUND_STREAM*, void*,
        int, void* self)
    {
        curSong = 0;
        static_cast<StreamData*>(self)->handle = -1;
        return 0;
    }

public:
    StreamData(Gosu::Reader reader)
    : stream(0), handle(-1)
    {
        buffer.resize(reader.resource().size() - reader.position());
        reader.read(&buffer[0], buffer.size());

// Disabled for licensing reasons.
// If you have a license to play MP3 files, compile with GOSU_ALLOW_MP3.
#ifndef GOSU_ALLOW_MP3
        if (buffer.size() > 2 &&
            ((buffer[0] == '\xff' && (buffer[1] & 0xfe) == '\xfa') ||
            (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3')))
        {
            throw std::runtime_error("MP3 file playback not allowed");
        }
#endif

        stream = FSOUND_Stream_Open(&buffer[0], FSOUND_LOADMEMORY | FSOUND_LOOP_NORMAL,
            0, buffer.size());
        if (stream == 0)
            throwLastFMODError();
        
        FSOUND_Stream_SetEndCallback(stream, endSongCallback, this);
    }
    
    ~StreamData()
    {
        if (stream != 0)
            FSOUND_Stream_Close(stream);
    }
    
    void play(bool looping)
    {
        if (handle == -1)
        {
            handle = FSOUND_Stream_Play(FSOUND_FREE, stream);
                FSOUND_Stream_SetLoopCount(stream, looping ? -1 : 0);
        }
        else if (paused())
            FSOUND_SetPaused(handle, 0);
        applyVolume();
    }
    
    void pause()
    {
        if (handle != -1)
            FSOUND_SetPaused(handle, 1);
    }
    
    bool paused() const
    {
        return handle != -1 && FSOUND_GetPaused(handle);
    }
    
    void stop()
    {
        fmodCheck(FSOUND_Stream_Stop(stream));
        handle = -1; // The end callback is NOT being called!
    }
    
    void applyVolume()
    {
        if (handle != -1)
            FSOUND_SetVolume(handle, static_cast<int>(volume() * 255));
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
};

Gosu::Song::Song(const std::wstring& filename)
{
    requireFMOD();
    
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
	Song(type, buf.frontReader()).data.swap(data);
}

Gosu::Song::Song(Type type, Reader reader)
{
    requireFMOD();
    
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

void Gosu::Song::update()
{
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
