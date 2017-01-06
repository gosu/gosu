#pragma once

#include "AudioFile.hpp"
#include <Gosu/IO.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

// Based on the Drama Sound Engine for D

namespace Gosu
{
    class OggFile : public AudioFile
    {
        Gosu::Buffer contents_;
        Gosu::Buffer buffer_;
        int channels_;
        ALenum sample_rate_;
        stb_vorbis* stream_;

        void open()
        {
            int error = 0;
            
            const unsigned char* mem = static_cast<const unsigned char*>(contents_.data());
            stream_ = stb_vorbis_open_memory(mem, contents_.size(), &error, 0);
            
            if (stream_ == 0) {
                throw std::runtime_error("Cannot open Ogg Vorbis file, error code: " +
                                         std::to_string(error));
            }
            
            stb_vorbis_info info = stb_vorbis_get_info(stream_);
            channels_ = info.channels;
            sample_rate_ = info.sample_rate;
            buffer_.resize(info.temp_memory_required);
        }
        
        void close()
        {
            stb_vorbis_close(stream_);
            stream_ = 0;
        }
        
    public:
        OggFile(Gosu::Reader reader)
        {
            contents_.resize(reader.resource().size() - reader.position());
            reader.read(contents_.data(), contents_.size());
            
            open();
        }
        
        ~OggFile()
        {
            close();
        }
        
        ALenum format() const
        {
            return (channels_ == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
        }
        
        ALuint sample_rate() const
        {
            return sample_rate_;
        }
        
        std::size_t read_data(void* dest, std::size_t length)
        {
            int samples = 0;
            int max_samples = length / sizeof(short);
            
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
        
        void rewind()
        {
            stb_vorbis_seek_start(stream_);
        }
    };
}
