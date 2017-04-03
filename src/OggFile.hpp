#pragma once

#include "AudioFile.hpp"
#include <Gosu/IO.hpp>
#include <stdexcept>
#include <string>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

namespace Gosu
{
    class OggFile : public AudioFile
    {
        Gosu::Buffer contents_;
        int channels_;
        ALenum sample_rate_;
        stb_vorbis* stream_;

    public:
        OggFile(Gosu::Reader reader)
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
        
        ~OggFile() override
        {
            stb_vorbis_close(stream_);
            stream_ = nullptr;
        }
        
        ALenum format() const override
        {
            return (channels_ == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
        }
        
        ALuint sample_rate() const override
        {
            return sample_rate_;
        }
        
        std::size_t read_data(void* dest, std::size_t length) override
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
        
        void rewind() override
        {
            stb_vorbis_seek_start(stream_);
        }
    };
}
