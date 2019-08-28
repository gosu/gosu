#pragma once

#include "AudioFile.hpp"
#include <Gosu/IO.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

#include <mpg123.h>

namespace Gosu
{
    class MPEGFile : public AudioFile
    {
        Gosu::Buffer contents_;
        off_t position_ = 0;
        mpg123_handle* handle_;
        
        enum {
            SAMPLE_RATE = 44100,
            INPUT_SIZE = 16384
        };
        
    public:
        explicit MPEGFile(Gosu::Reader reader)
        {
            contents_.resize(reader.resource().size() - reader.position());
            reader.read(contents_.data(), contents_.size());

            static int init_error = mpg123_init();
            if (init_error != MPG123_OK) {
                throw std::runtime_error("Cannot initialize mpg123: " +
                                         std::string(mpg123_plain_strerror(init_error)));
            }

            int new_error;
            handle_ = mpg123_new(nullptr, &new_error);
            if (handle_ == nullptr) {
                throw std::runtime_error("Cannot initialize mpg123 decoder: " +
                                         std::string(mpg123_plain_strerror(new_error)));
            }
            
            mpg123_param(handle_, MPG123_FORCE_RATE, SAMPLE_RATE, 0);
            mpg123_param(handle_, MPG123_FLAGS, MPG123_FORCE_STEREO, 0);
            
            mpg123_open_feed(handle_);
        }
        
        ~MPEGFile() override
        {
            mpg123_delete(handle_);
        }
        
        ALenum format() const override
        {
            return AL_FORMAT_STEREO16;
        }
        
        ALuint sample_rate() const override
        {
            return SAMPLE_RATE;
        }
        
        double length() const override
        {
            return mpg123_length(handle_) / SAMPLE_RATE;
        }

        std::size_t read_data(void* dest, std::size_t length) override
        {
            std::size_t written = 0;
            int error = 0;
            
            error = mpg123_read(handle_, static_cast<unsigned char*>(dest), length, &written);
            while (written == 0
                   && (error == MPG123_NEED_MORE || error == MPG123_NEW_FORMAT)
                   && position_ != contents_.size()) {
                auto bytes = std::min<std::size_t>(contents_.size() - position_, INPUT_SIZE);
                // (Not sure what to do about the return value of mpg123_feed here.)
                mpg123_feed(handle_, static_cast<unsigned char*>(contents_.data()) + position_,
                            bytes);
                position_ += bytes;
                error = mpg123_read(handle_, static_cast<unsigned char*>(dest), length, &written);
            }
            
            return written;
        }
        
        void rewind() override
        {
            mpg123_feedseek(handle_, 0, SEEK_SET, &position_);
            assert (position_ >= 0 && position_ <= contents_.size());
        }
    };
}
