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

#include <SDL.h>
#include <SDL_mixer.h>

namespace Gosu {
  namespace {
    GOSU_NORETURN void throwLastSDLError() {
      throw std::runtime_error(Mix_GetError());
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

  Mix_CloseAudio();
  mixerInitialized = false;
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
}

void Gosu::SampleInstance::stop()
{
}

void Gosu::SampleInstance::changeVolume(double volume)
{
  Mix_Volume(channel, boundBy<int>(volume * 255, 0, 255));
}

void Gosu::SampleInstance::changePan(double pan)
{
  int leftPan = boundBy<int>(pan * 127, 0, 127);
  Mix_SetPanning(channel, leftPan, 254 - leftPan);
}

void Gosu::SampleInstance::changeSpeed(double speed)
{
  /* We ignore the speed for now as this seems to be non-trivial
   * with SDL_mixer. */
}

struct Gosu::Sample::SampleData : boost::noncopyable
{
  Mix_Chunk* rep;
  
  SampleData(): rep(0) {}
  ~SampleData() {
    if (rep != 0) Mix_FreeChunk(rep);
  }
};

Gosu::Sample::Sample(Audio& audio, const std::wstring& filename)
{
  Buffer buf;
  loadFile(buf, filename);

  // Forward.
  Sample(audio, buf.frontReader()).data.swap(data);
}

Gosu::Sample::Sample(Audio& audio, Reader reader) {
  std::size_t bufsize = reader.resource().size() - reader.position();
  Uint8* buffer = new Uint8[bufsize];
  reader.read(buffer, bufsize);
  
  data.reset(new SampleData);
  data->rep = Mix_LoadWAV_RW(SDL_RWFromMem(buffer, bufsize), 1);
  if (data->rep == NULL)
    throwLastSDLError();
}

Gosu::Sample::~Sample() {
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed) const {
  int channel = Mix_PlayChannel(-1, data->rep, 0);
  int extra;
  SampleInstance result(channel, extra);
  
  if (channel > 0) {
    Mix_SetPanning(channel, 127, 127);

    if (volume != 1)
        result.changeVolume(volume);

    /* We ignore the speed for now as this seems to be non-trivial
     * with SDL_mixer. */
  }
  
  return result;
}

Gosu::SampleInstance Gosu::Sample::playPan(double pan, double volume,
    double speed) const {
  int channel = Mix_PlayChannel(-1, data->rep, 0);
  int extra;
  SampleInstance result(channel, extra);

  if (channel > 0) {
    result.changePan(pan);

    if (volume != 1)
      result.changeVolume(volume);

    /* Speed ignored for now, as above. */
  }
  
  return result;
}

// No class hierarchy here; SDL_mixer abstracts this away for us.
class Gosu::Song::BaseData : boost::noncopyable {
public:
    Mix_Music* music;
    std::vector<Uint8> buffer;
    int volume;
    
    BaseData() : music(0), volume(0) {}
    ~BaseData() {
      if (music) Mix_FreeMusic(music);
    }

    static void endSongCallback() {
        curSong = 0;
    }
};

    void play() {
      if (Mix_PlayMusic(music, 0) < 0)
        throwLastSDLError();
    }

    void stop() {
          }
};

Gosu::Song::Song(Audio& audio, const std::wstring& filename)
{
  Buffer buf;
  loadFile(buf, filename);
  Song(audio, type, buf.frontReader()).data.swap(data);
}

Gosu::Song::Song(Audio &audio, Type type, Reader reader)
: data(new BaseData)
{
    std::size_t bufsize = reader.resource().size() - reader.position();
    pimpl->buffer.resize(bufsize);
    reader.read(buffer.data(), bufsize);
// Disabled for licensing reasons.
// If you have a license to play MP3 files, compile with GOSU_ALLOW_MP3.
#ifndef GOSU_ALLOW_MP3
    if (bufsize > 2 && 
      ((pimpl->buffer[0] == '\xff' && (pimpl->buffer[1] & 0xfe) == '\xfa') || 
       (pimpl->buffer[0] == 'I' && pimpl->buffer[1] == 'D' && pimpl->buffer[2] == '3')))
    throw std::runtime_error("MP3 playback not allowed.");
#endif
    data->music = Mix_LoadMUS_RW(SDL_RWFromMem(pimpl->buffer.data(), bufsize));
    if (data->music == NULL)
        throwLastSDLError();

    Mix_HookMusicFinished(BaseData::endSongCallback);
}

Gosu::Song::~Song() {
  stop();
}

void Gosu::Song::play() {
  if (curSong)
    curSong->stop();
  
  assert(curSong == 0);

  Mix_MusicVolume(data->volume);
  data->play();
  curSong = this;
}

void Gosu::Song::stop() {
  if (playing()) {
    Mix_HaltMusic();
    curSong = 0;
  }
}

bool Gosu::Song::playing() const {
  return curSong == this;
}

void Gosu::Song::changeVolume(double volume) {
  data->volume = boundBy<int>(volume * MIX_MAX_VOLUME, 0, MIX_MAX_VOLUME);
  if (playing())
    Mix_MusicVolume(data->volume);
}
