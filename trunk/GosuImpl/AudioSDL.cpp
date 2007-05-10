#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <SDL.h>
#include <SDL_mixer.h>

namespace Gosu {
  namespace {
    GOSU_NORETURN void throwLastSDLError() {
      throw std::runtime_error(::Mix_GetError());
    }

    bool mixerInitialized = false;
    Song* curSong = NULL;
  }
}

#ifdef GOSU_IS_WIN
Gosu::Audio::Audio(HWND window) {
  // Nothing to do; SDL_mixer does not require the window handle.
#else
Gosu::Audio::Audio() {
#endif
  if (mixerInitialized)
    throw std::logic_error("Multiple Gosu::Audio instances not supported.");

  if (SDL_Init(SDL_INIT_AUDIO) < 0)
    throw std::runtime_error("Unable to initialize SDL audio.");

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
    throwLastSDLError();

  mixerInitialized = true;
}

Gosu::Audio::~Audio() {
  assert(mixerInitialized);

  ::Mix_CloseAudio();
  mixerInitialized = false;
}

struct Gosu::Sample::SampleData : boost::noncopyable
{
  Mix_Chunk* rep;
  
  SampleData(): rep(0) {}
  ~SampleData() {
    if (rep != 0) ::Mix_FreeChunk(rep);
  }
};

Gosu::Sample::Sample(Audio& audio, Reader reader) {
  std::size_t bufsize = reader.resource().size() - reader.position();
  Uint8* buffer = new Uint8[bufsize];
  reader.read(buffer, bufsize);
  
  data.reset(new SampleData);
  data->rep = ::Mix_LoadWAV_RW(::SDL_RWFromMem(buffer, bufsize), 1);
  if (data->rep == NULL)
    throwLastSDLError();

  //if (buffer != 0)
  //  delete[] buffer; // XXX: does SDL_mixer use this data?
}

Gosu::Sample::~Sample() {
}

void Gosu::Sample::play(double volume, double speed) const {
  int channel = ::Mix_PlayChannel(-1, data->rep, 0);
  
  if (channel > 0) {
    ::Mix_SetPanning(channel, 127, 127);

    if (volume != 1)
      ::Mix_Volume(channel, trunc(boundBy(volume * 255, 0.0, 255.0)));

    /* We ignore the speed for now as this seems to be non-trivial
     * with SDL_mixer. */
  }
}

void Gosu::Sample::playPan(double pan, double volume, double speed) const {
  int channel = ::Mix_PlayChannel(-1, data->rep, 0);

  if (channel > 0) {
    Uint8 pan = trunc(boundBy(pan * 127, 0, 127));
    ::Mix_SetPanning(channel, pan, 254-pan);

    if (volume != 1)
      ::Mix_Volume(channel, trunc(boundBy(volume * 255, 0.0, 255.0)));

    /* Speed ignored for now, as above. */
  }
}

class Gosu::Song::BaseData : boost::noncopyable {
  public:
    virtual ~BaseData() {}

    virtual void play() = 0;
    virtual void stop() = 0;
};

class Gosu::Song::StreamData : public BaseData {
  Mix_Music* music;
  Uint8* buffer;

  void endSongCallback() {
    curSong = 0;
  }

  public:
    StreamData(Gosu::Reader reader)
      : music(0)
    {
#if 0 // no Mix_LoadMUS_RW :|
      std::size_t bufsize = reader.resource().size() - reader.position();
      buffer = new Uint8[bufsize];
      reader.read(buffer, bufsize);
#ifdef GOSU_ALLOW_MP3
      if (bufsize > 2 && 
          ((buffer[0] == '\xff' && (buffer[1] & 0xfe) == '\xfa') || 
           (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3')))
        throw std::runtime_error("MP3 playback not allowed.");
#endif
      music = ::Mix_LoadMUS_RW(::SDL_RWFromMem(buffer, bufsize));
      if (music == NULL)
        throwLastSDLError();

      ::Mix_HookMusicFinished(endSongCallback);
#endif
    }

    ~StreamData() {
      if (music != NULL)
        Mix_FreeMusic(music);
      if (buffer != NULL)
        delete[] buffer;
    }

    void play() {
      if (::Mix_PlayMusic(music, 0) < 0)
        throwLastSDLError();
    }

    void stop() {
      ::Mix_HaltMusic();
    }
};

class Gosu::Song::ModuleData : public Gosu::Song::StreamData {
  public:
    ModuleData(Gosu::Reader reader): StreamData(reader) {}
};

Gosu::Song::Song(Audio &audio, Type type, Reader reader) {
  switch (type) {
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

Gosu::Song::~Song() {
  stop();
}

void Gosu::Song::play() {
  if (curSong)
    curSong->stop();
  
  assert(curSong == 0);

  data->play();
  curSong = this;
}

void Gosu::Song::stop() {
  if (playing()) {
    data->stop();
    curSong = 0;
  }
}

bool Gosu::Song::playing() const {
  return curSong == this;
}
