#ifndef GOSUIMPL_AUDIO_AUDIOFILE_HPP
#define GOSUIMPL_AUDIO_AUDIOFILE_HPP

#include <boost/noncopyable.hpp>
#include <vector>
#import <OpenAL/al.h>

namespace Gosu
{
    class AudioFile : boost::noncopyable
    {
    public:
        virtual ~AudioFile() {}
        virtual ALenum format() const = 0;
        virtual ALuint sampleRate() const = 0;
        virtual std::size_t readData(void* dest, std::size_t length) = 0;
        virtual void rewind() = 0;
        virtual const std::vector<char>& decodedData() = 0;
    };
}

#endif
