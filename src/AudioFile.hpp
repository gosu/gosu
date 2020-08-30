#pragma once

#include <Gosu/IO.hpp>

#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif

#include <cstddef>
#include <string>
#include <vector>

namespace Gosu
{
    class AudioFile
    {
        struct Impl;
        std::unique_ptr<Impl> pimpl;
        
    public:
        explicit AudioFile(const std::string& filename);
        explicit AudioFile(Reader reader);
        ~AudioFile();

        ALenum format() const;
        
        ALuint sample_rate() const;
        
        std::size_t read_data(void* dest, std::size_t length);
        
        void rewind();
        
        const std::vector<char>& decoded_data();
    };
}
