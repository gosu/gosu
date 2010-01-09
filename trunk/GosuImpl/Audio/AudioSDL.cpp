#include <Gosu/Audio.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>

#include <SDL.h>
#include <SDL_mixer.h>

namespace Gosu {
  namespace {
    GOSU_NORETURN void throwLastSDLError() {
      throw std::runtime_error(Mix_GetError());
    }

    bool noSound = false;
    Song* curSong = NULL;

    void requireSDLMixer()
    {
      static bool mixerInitialized = false;
      if (mixerInitialized)
          return;
      if (SDL_Init(SDL_INIT_AUDIO) || Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
          noSound = true;
      std::atexit(Mix_CloseAudio);
      mixerInitialized = true;
    }
  }
  
  std::map<int, int> channelRegistry;
}

bool Gosu::SampleInstance::alive() const
{
    return !noSound && channelRegistry[handle] == extra &&
        Mix_Playing(handle) == 1;
}

Gosu::SampleInstance::SampleInstance(int handle, int extra)
: handle(handle), extra(extra)
{
}

bool Gosu::SampleInstance::playing() const
{
    return alive() && !Mix_Paused(handle);
}

void Gosu::SampleInstance::resume()
{
    if (alive())
        Mix_Resume(handle);
}

void Gosu::SampleInstance::pause()
{
    if (alive())
        Mix_Pause(handle);
}

bool Gosu::SampleInstance::paused() const
{
    return alive() && Mix_Paused(handle);
}

void Gosu::SampleInstance::stop()
{
    if (alive())
        Mix_HaltChannel(handle);
}

void Gosu::SampleInstance::changeVolume(double volume)
{
    if (alive())
        Mix_Volume(handle, clamp<int>(volume * 255, 0, 255));
}

void Gosu::SampleInstance::changePan(double pan)
{
    int rightPan = clamp<int>(pan * 127, 0, 127);
    if (alive())
        Mix_SetPanning(handle, 254 - rightPan, rightPan);
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

Gosu::Sample::Sample(const std::wstring& filename)
{
    requireSDLMixer();

    if (noSound)
        return;

    data.reset(new SampleData);
    // Saved locally because Mix_LoadWAV is a macro, wouldn't trust it...
    std::string filenameUTF8 = wstringToUTF8(filename);
    data->rep = Mix_LoadWAV(filenameUTF8.c_str());
    if (data->rep == NULL)
        throwLastSDLError();
}

Gosu::Sample::Sample(Reader reader)
{
    requireSDLMixer();

    if (noSound)
        return;

    std::size_t bufsize = reader.resource().size() - reader.position();
    Uint8* buffer = static_cast<Uint8*>(malloc(bufsize));
    reader.read(buffer, bufsize);

    data.reset(new SampleData);
    data->rep = Mix_LoadWAV_RW(SDL_RWFromMem(buffer, bufsize), 1);
    if (data->rep == NULL)
        throwLastSDLError();
}

Gosu::Sample::~Sample() {
}

Gosu::SampleInstance Gosu::Sample::play(double volume, double speed,
    bool looping) const {
  int channel;
  
  if (noSound || (channel = Mix_PlayChannel(-1, data->rep, looping ? -1 : 0)) == -1)
    return SampleInstance(-1, -1);

  int extra = ++channelRegistry[channel];
  SampleInstance result(channel, extra);

  if (volume != 1)
      result.changeVolume(volume);

  return result;

  /* We ignore the speed for now as this seems to be non-trivial
   * with SDL_mixer. */
}

Gosu::SampleInstance Gosu::Sample::playPan(double pan, double volume,
    double speed, bool looping) const {
  int channel;
  
  if (noSound || (channel = Mix_PlayChannel(-1, data->rep, looping ? -1 : 0)) == -1)
    return SampleInstance(-1, -1);

  int extra = ++channelRegistry[channel];
  SampleInstance result(channel, extra);

  result.changePan(pan);

  if (volume != 1)
    result.changeVolume(volume);

  return result;

  /* Speed ignored for now, as above. */
}

// No class hierarchy here; SDL_mixer abstracts this away for us.
class Gosu::Song::BaseData : boost::noncopyable {
public:
    Mix_Music* music;
    //std::vector<Uint8> buffer; - used by constructor that has been commented out
    double volume;
    
    BaseData() : music(0), volume(1.0) {}
    ~BaseData() {
      if (music) Mix_FreeMusic(music);
    }

    static void endSongCallback() {
        curSong = 0;
    }
};

Gosu::Song::Song(const std::wstring& filename)
: data(new BaseData)
{
    requireSDLMixer();

    if (noSound)
        return;

    data->music = Mix_LoadMUS(Gosu::wstringToUTF8(filename).c_str());
    if (data->music == NULL)
        throwLastSDLError();

    Mix_HookMusicFinished(BaseData::endSongCallback);
}

Gosu::Song::Song(Type type, Reader reader)
: data(new BaseData)
{
    requireSDLMixer();

    if (noSound)
        return;
#if 0
    // This is traditionally broken in SDL_mixer. File bugs :)
    
    std::size_t bufsize = reader.resource().size() - reader.position();
    data->buffer.resize(bufsize);
    reader.read(data->buffer.data(), bufsize);
    data->music = Mix_LoadMUS_RW(SDL_RWFromMem(data->buffer.data(), bufsize));
    if (data->music == NULL)
        throwLastSDLError();

    Mix_HookMusicFinished(BaseData::endSongCallback);
#else
    throw std::runtime_error("Loading files from memory not possible with libSDL_mixer");
#endif
}

Gosu::Song::~Song() {
  stop();
}

Gosu::Song* Gosu::Song::currentSong() {
  return curSong;
}

void Gosu::Song::play(bool looping) {
  if (noSound)
    return;

  if (curSong && curSong != this)
  {
      curSong->stop();
      assert(curSong == 0);
  }

  if (Mix_PausedMusic())
    Mix_ResumeMusic();
  if (!playing() && Mix_PlayMusic(data->music, looping ? -1 : 0) < 0)
    throwLastSDLError();
  changeVolume(data->volume);
  curSong = this;
}

void Gosu::Song::pause()
{
    if (curSong == this)
        Mix_PauseMusic();
}

bool Gosu::Song::paused() const
{
    return curSong == this && Mix_PausedMusic();
}

void Gosu::Song::stop() {
  if (curSong == this) {
    Mix_HaltMusic();
    curSong = 0;
  }
}

bool Gosu::Song::playing() const {
  return curSong == this && !paused();
}

double Gosu::Song::volume() const {
  return data->volume;
}

void Gosu::Song::changeVolume(double volume) {
  data->volume = clamp(volume, 0.0, 1.0);

  if (playing())
    Mix_VolumeMusic(trunc(data->volume * MIX_MAX_VOLUME));
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
