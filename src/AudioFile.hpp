#pragma once

#include <Gosu/IO.hpp>

#ifdef GOSU_IS_IPHONE
// Ignore OpenAL deprecation warnings. If macOS stops shipping OpenAL, it's more likely that we bundle our own version
// of it than that we switch to another audio API.
#define OPENAL_DEPRECATED
#include <OpenAL/al.h>
#else
#include <al.h>
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
