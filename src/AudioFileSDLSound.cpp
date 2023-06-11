#include <Gosu/Platform.hpp>

#ifndef GOSU_IS_IPHONE

#include "AudioFile.hpp"
#include "AudioImpl.hpp"

#include <SDL_sound.h>

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>
#include <stdexcept>

struct Gosu::AudioFile::Impl : private Gosu::Noncopyable
{
    Buffer buffer;

    std::shared_ptr<Sound_Sample> sample;

    std::vector<char> data;

    Impl()
    {
        static std::once_flag initialized;
        std::call_once(initialized, Sound_Init);
        // Don't bother calling Sound_Quit(); it's hard to know when to clean up within the scope
        // of a Ruby C extension. Let the operating system clean up after us.
    }
};

Gosu::AudioFile::AudioFile(const std::string& filename)
: pimpl(new Impl)
{
    // We need to set a "desired" audio format, otherwise we may encounter data formats such as
    // floating-point samples that Apple's OpenAL implementation does not support.
    Sound_AudioInfo desired;
    // Prefer 16-bit signed data which is supported by all versions of OpenAL.
    desired.format = AUDIO_S16;
    // 0 means "use whatever we have" (see SDL_sound.c, init_sample).
    desired.channels = 0;
    desired.rate = 0;

    pimpl->sample.reset(Sound_NewSampleFromFile(filename.c_str(), &desired, 4096),
                        Sound_FreeSample);
    if (!pimpl->sample) {
        std::string message = "Could not parse audio file " + filename;
        if (const char* error = Sound_GetError()) {
            message += ": ";
            message += error;
        }
        throw std::runtime_error(message);
    }
}

Gosu::AudioFile::AudioFile(Reader reader)
: pimpl(new Impl)
{
    pimpl->buffer.resize(reader.resource().size() - reader.position());
    reader.read(pimpl->buffer.data(), pimpl->buffer.size());
    pimpl->sample.reset(Sound_NewSampleFromMem(reinterpret_cast<Uint8*>(pimpl->buffer.data()),
                                               static_cast<Uint32>(pimpl->buffer.size()),
                                               "", nullptr, 4096),
                        Sound_FreeSample);
    if (!pimpl->sample) {
        std::string message = "Could not parse audio file";
        if (const char* error = Sound_GetError()) {
            message += ": ";
            message += error;
        }
        throw std::runtime_error(message);
    }
}

Gosu::AudioFile::~AudioFile()
{
}

ALenum Gosu::AudioFile::format() const
{
    auto channels = pimpl->sample->desired.channels;
    auto format = pimpl->sample->desired.format;

    if (channels == 1 && SDL_AUDIO_ISINT(format)) {
        if (SDL_AUDIO_BITSIZE(format) == 8) return AL_FORMAT_MONO8;
        if (SDL_AUDIO_BITSIZE(format) == 16) return AL_FORMAT_MONO16;
    }
    if (channels == 2 && SDL_AUDIO_ISINT(format)) {
        if (SDL_AUDIO_BITSIZE(format) == 8) return AL_FORMAT_STEREO8;
        if (SDL_AUDIO_BITSIZE(format) == 16) return AL_FORMAT_STEREO16;
    }

    throw std::runtime_error("Unsupported number of channels: " + std::to_string(channels) +
                             " and format: " + std::to_string(format));
}

ALuint Gosu::AudioFile::sample_rate() const
{
    return pimpl->sample->actual.rate;
}

void Gosu::AudioFile::rewind()
{
    Sound_Rewind(pimpl->sample.get());
}

std::size_t Gosu::AudioFile::read_data(void* dest, size_t length)
{
    std::size_t read = 0;
    while (length > 0) {
        // TODO offset magic
        auto result = static_cast<std::size_t>(Sound_Decode(pimpl->sample.get()));
        if (result == 0) break;
        auto to_copy = std::min(length, result);
        std::memcpy(dest, pimpl->sample->buffer, to_copy);
        dest = static_cast<char*>(dest) + to_copy;
        length -= to_copy;
        read += to_copy;
    }
    return read;
}

const std::vector<char>& Gosu::AudioFile::decoded_data()
{
   if (!pimpl->data.empty()) {
       return pimpl->data;
   }

   static const std::size_t INCREMENT = 512 * 1024;

   for (;;) {
       pimpl->data.resize(pimpl->data.size() + INCREMENT);

       auto read_bytes = read_data(&pimpl->data[pimpl->data.size() - INCREMENT], INCREMENT);

       if (read_bytes < INCREMENT) {
           pimpl->data.resize(pimpl->data.size() - INCREMENT + read_bytes);
           break;
       }
   }

   return pimpl->data;
}

#endif
