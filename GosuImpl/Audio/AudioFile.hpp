#ifndef GOSUIMPL_AUDIO_AUDIOFILE_HPP
#define GOSUIMPL_AUDIO_AUDIOFILE_HPP

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
        
        std::vector<char> decodedData_;
        
    public:
        AudioFile() {}
        virtual ~AudioFile() {}
        virtual ALenum format() const = 0;
        virtual ALuint sampleRate() const = 0;
        virtual std::size_t readData(void* dest, std::size_t length) = 0;
        virtual void rewind() = 0;
        
        const std::vector<char>& decodedData()
        {
            static const unsigned INCREMENT = 512*1024;
            
            if (!decodedData_.empty())
                return decodedData_;
            
            for (;;)
            {
                decodedData_.resize(decodedData_.size() + INCREMENT);
                int readBytes = readData(&decodedData_[decodedData_.size() - INCREMENT],
                                    INCREMENT);
                if (readBytes < INCREMENT)
                {
                    decodedData_.resize(decodedData_.size() - INCREMENT + readBytes);
                    break;
                }
            }
            
            return decodedData_;
        }
    };
}

#endif
