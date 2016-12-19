#pragma once

#include <vector>
#include <Gosu/Platform.hpp>
#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif

namespace Gosu
{
    class AudioFile
    {
        AudioFile(const AudioFile&);
        AudioFile& operator=(const AudioFile&);
        
        std::vector<char> decoded_data_;
        
    public:
        AudioFile() {}
        virtual ~AudioFile() {}
        virtual ALenum format() const = 0;
        virtual ALuint sample_rate() const = 0;
        virtual std::size_t read_data(void* dest, std::size_t length) = 0;
        virtual void rewind() = 0;
        
        const std::vector<char>& decoded_data()
        {
            static const unsigned INCREMENT = 512*1024;
            
            if (!decoded_data_.empty())
                return decoded_data_;
            
            for (;;)
            {
                decoded_data_.resize(decoded_data_.size() + INCREMENT);
                int read_bytes = read_data(&decoded_data_[decoded_data_.size() - INCREMENT],
                                    INCREMENT);
                if (read_bytes < INCREMENT)
                {
                    decoded_data_.resize(decoded_data_.size() - INCREMENT + read_bytes);
                    break;
                }
            }
            
            return decoded_data_;
        }
    };
}
