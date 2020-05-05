#pragma once

#include <Gosu/Platform.hpp>
#include <vector>
#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif

namespace Gosu
{
    class AudioFile
    {
        AudioFile(const AudioFile&) = delete;
        AudioFile& operator=(const AudioFile&) = delete;
        AudioFile(AudioFile&&) = delete;
        AudioFile& operator=(AudioFile&&) = delete;
        
        std::vector<char> data_;
        
    public:
        AudioFile() = default;
        
        virtual ~AudioFile() = default;
        
        virtual ALenum format() const = 0;
        
        virtual ALuint sample_rate() const = 0;
        
        virtual double length() const = 0;

        virtual std::size_t read_data(void* dest, std::size_t length) = 0;
        
        virtual void rewind() = 0;
        
        const std::vector<char>& decoded_data()
        {
            static const std::size_t INCREMENT = 512 * 1024;
            
            if (!data_.empty()) {
                return data_;
            }
            
            for (;;) {
                data_.resize(data_.size() + INCREMENT);
                
                auto read_bytes = read_data(&data_[data_.size() - INCREMENT], INCREMENT);
                
                if (read_bytes < INCREMENT) {
                    data_.resize(data_.size() - INCREMENT + read_bytes);
                    break;
                }
            }
            
            return data_;
        }
    };
}
