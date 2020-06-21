#include "OggFile.hpp"

// Disable comma warnings in stb headers.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomma"
#endif

#include <stb_vorbis.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

Gosu::OggFile::OggFile(Gosu::Reader reader)
{
    contents_.resize(reader.resource().size() - reader.position());
    reader.read(contents_.data(), contents_.size());
    
    int error = 0;
    
    auto data = static_cast<const unsigned char*>(contents_.data());
    auto size = static_cast<int>(contents_.size());
    stream_ = stb_vorbis_open_memory(data, size, &error, 0);
    
    if (stream_ == nullptr) {
        throw std::runtime_error("Cannot open Ogg Vorbis file, error code: " +
                                 std::to_string(error));
    }
    
    stb_vorbis_info info = stb_vorbis_get_info(stream_);
    channels_ = info.channels;
    sample_rate_ = info.sample_rate;
}

Gosu::OggFile::~OggFile()
{
    stb_vorbis_close(stream_);
    stream_ = nullptr;
}

ALenum Gosu::OggFile::format() const
{
    return (channels_ == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
}

ALuint Gosu::OggFile::sample_rate() const
{
    return sample_rate_;
}

std::size_t Gosu::OggFile::read_data(void* dest, std::size_t length)
{
    int samples = 0;
    int max_samples = static_cast<int>(length / sizeof(short));
    
    while (samples < max_samples) {
        int samples_per_channel =
            stb_vorbis_get_samples_short_interleaved(stream_, channels_,
                                                     static_cast<short*>(dest) + samples,
                                                     max_samples - samples);
        
        if (samples_per_channel == 0) break;
        
        samples += samples_per_channel * channels_;
    }
    
    return samples * sizeof(short);
}

void Gosu::OggFile::rewind()
{
    stb_vorbis_seek_start(stream_);
}
