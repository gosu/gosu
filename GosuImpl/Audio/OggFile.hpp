#ifndef GOSUIMPL_AUDIO_OGGFILE_HPP
#define GOSUIMPL_AUDIO_OGGFILE_HPP

#include <GosuImpl/Audio/AudioFile.hpp>
#include <Gosu/IO.hpp>
#include <vorbis/vorbisfile.h>
#include <cassert>
#include <algorithm>
#include <stdexcept>

// Based on the Drama Sound Engine for D

namespace Gosu
{
    class OggFile : public AudioFile
    {
        Gosu::Buffer buffer_;
        Gosu::Reader reader_;
        ALenum format_;
        ALenum sampleRate_;
        OggVorbis_File file_;

        // extern "C"
        static std::size_t readCallback(void* ptr, std::size_t size, std::size_t nmemb, void* datasource)
        {
            OggFile* oggFile = static_cast<OggFile*>(datasource);
            size = std::min(size * nmemb,
                oggFile->buffer_.size() - oggFile->reader_.position());
            oggFile->reader_.read(ptr, size);
            return size;
        }
        
        void setup()
        {
            static const ov_callbacks cbs = { readCallback, 0, 0, 0 };
            ov_open_callbacks(this, &file_, 0, 0, cbs);
            
            vorbis_info* info = ov_info(&file_, -1); // -1 is current bitstream
            if (ov_streams(&file_) != 1)
                throw std::runtime_error("multi-stream vorbis files not supported");
            
            format_ = info->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            sampleRate_ = info->rate;
        }
        
        void teardown()
        {
            ov_clear(&file_);
        }
        
    public:
        OggFile(Gosu::Reader reader)
        : reader_(buffer_.frontReader())
        {
            buffer_.resize(reader.resource().size() - reader.position());
            reader.read(buffer_.data(), buffer_.size());
            
            setup();
        }
        
        ~OggFile()
        {
            teardown();
        }
        
        ALenum format() const
        {
            return format_;
        }
        
        ALuint sampleRate() const
        {
            return sampleRate_;
        }
        
        std::size_t readData(void* dest, std::size_t length)
        {
            static const unsigned OGG_ENDIANNESS =
            #ifdef GOSU_IS_BIG_ENDIAN
                1;
            #else
                0;
            #endif
            
            char* ptr = static_cast<char*>(dest);
            std::size_t size = 0;
            int result;
            int section;
            while (size < length)
            {
                result = ov_read(&file_, ptr + size, length - size,
                                 OGG_ENDIANNESS, 2 /* 16-bit */,
                                 1 /* signed */, &section);
                assert(section == 0);
                if (result > 0)
                    size += result;
                else
                    if (result < 0)
                        throw std::runtime_error("error reading vorbis stream");
                    else
                        break;
            }
            
            return size;
        }
        
        void rewind()
        {
            teardown();
            reader_ = buffer_.frontReader();
            setup();
        }
    };
}

#endif
