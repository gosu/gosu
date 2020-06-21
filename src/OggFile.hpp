#pragma once

#include "AudioFile.hpp"
#include <Gosu/IO.hpp>
#include <stdexcept>
#include <string>

typedef struct stb_vorbis stb_vorbis;

namespace Gosu
{
    class OggFile : public AudioFile
    {
        Gosu::Buffer contents_;
        int channels_;
        ALenum sample_rate_;
        stb_vorbis* stream_;

    public:
        OggFile(Gosu::Reader reader);
        
        ~OggFile() override;
        
        ALenum format() const override;
        
        ALuint sample_rate() const override;
        
        std::size_t read_data(void* dest, std::size_t length) override;
        
        void rewind() override;
    };
}
