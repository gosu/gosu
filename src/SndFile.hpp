#pragma once

#include <Gosu/Audio.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <sndfile.h>

#ifdef GOSU_IS_WIN
#define NOMINMAX
#include <windows.h>
#endif

namespace Gosu
{
    class SndFile : public AudioFile
    {
        SNDFILE* file;
        SF_INFO info;
        Reader reader;
        Buffer buffer;
        
        // Cannot use /DELAYLOAD with libsndfile.dll because it was compiled
        // using arcane GNU tools of dark magic (or maybe it's the filename).
        #ifdef GOSU_IS_WIN
        static HMODULE dll()
        {
            static HMODULE dll = LoadLibrary(L"libsndfile.dll");
            if (!dll) throw std::runtime_error("Cannot find libsndfile.dll");
            return dll;
        }
        
        #define CREATE_STUB(NAME, RETURN, PARAMS, NAMES)                    \
        static RETURN NAME PARAMS                                           \
        {                                                                   \
            typedef RETURN (__cdecl *NAME##_ptr) PARAMS;                    \
            static NAME##_ptr f = (NAME##_ptr)GetProcAddress(dll(), #NAME); \
            if (!f) {                                                       \
                throw std::runtime_error("Cannot find " ## #NAME);          \
            }                                                               \
            return f NAMES;                                                 \
        }
        CREATE_STUB(sf_open_virtual, SNDFILE*,
            (SF_VIRTUAL_IO* sfvirtual, int mode, SF_INFO* sfinfo, void* user_data),
            (sfvirtual, mode, sfinfo, user_data))
        CREATE_STUB(sf_close, int,
            (SNDFILE* sndfile),
            (sndfile))
        CREATE_STUB(sf_readf_short, sf_count_t,
            (SNDFILE* sndfile, short* ptr, sf_count_t items),
            (sndfile, ptr, items))
        CREATE_STUB(sf_seek, sf_count_t,
            (SNDFILE* sndfile, sf_count_t frames, int whence),
            (sndfile, frames, whence))
        CREATE_STUB(sf_strerror, const char*,
            (SNDFILE* sndfile),
            (sndfile))
        #undef CREATE_STUB
        #endif
        
        static sf_count_t get_filelen(SndFile* self)
        {
            return self->buffer.size();
        }
        
        static sf_count_t seek(sf_count_t offset, int whence, SndFile* self)
        {
            switch (whence) {
            case SEEK_SET:
                self->reader.set_position(offset);
                break;
            case SEEK_CUR:
                self->reader.seek(offset);
                break;
            case SEEK_END:
                self->reader.set_position(self->buffer.size() - offset);
                break;
            }

            if (self->reader.position() > self->buffer.size()) {
                self->reader.set_position(self->buffer.size());
            }
            
            return 0;
        }
        
        static sf_count_t read(void* ptr, sf_count_t count, SndFile* self)
        {
            sf_count_t avail = self->buffer.size() - self->reader.position();
            count = std::min(avail, count);
            self->reader.read(ptr, count);
            return count;
        }
        
        static sf_count_t tell(SndFile* self)
        {
            return self->reader.position();
        }
        
        static SF_VIRTUAL_IO* io_interface()
        {
            static SF_VIRTUAL_IO io;
            io.get_filelen = (sf_vio_get_filelen)&get_filelen;
            io.seek        = (sf_vio_seek)&seek;
            io.read        = (sf_vio_read)&read;
            io.tell        = (sf_vio_tell)&tell;
            io.write       = nullptr;
            return &io;
        }
        
    public:
        SndFile(Reader reader)
        : file(nullptr), reader(buffer.front_reader())
        {
            info.format = 0;
            buffer.resize(reader.resource().size() - reader.position());
            reader.read(buffer.data(), buffer.size());
            file = sf_open_virtual(io_interface(), SFM_READ, &info, this);
            if (!file) {
                throw std::runtime_error(sf_strerror(nullptr));
            }
        }
        
        SndFile(const std::string& filename)
        : file(nullptr), reader(buffer.front_reader())
        {
            info.format = 0;
            // TODO: Not sure if this is still necessary.
            // Can libsndfile open UTF-8 filenames on Windows?
            #ifdef GOSU_IS_WIN
            load_file(buffer, filename);
            file = sf_open_virtual(io_interface(), SFM_READ, &info, this);
            #else
            file = sf_open(filename.c_str(), SFM_READ, &info);
            #endif
            if (!file) {
                throw std::runtime_error(sf_strerror(nullptr));
            }
        }
        
        ~SndFile()
        {
            if (file) {
                sf_close(file);
            }
        }
        
        ALenum format() const
        {
            switch (info.channels) {
            case 1:
                return AL_FORMAT_MONO16;
            case 2:
                return AL_FORMAT_STEREO16;
            default:
                throw std::runtime_error("Too many channels in audio file");
            };
        }
        
        ALuint sample_rate() const
        {
            return info.samplerate;
        }
        
        std::size_t read_data(void* dest, std::size_t length)
        {
            int frame_size = sizeof(short) * info.channels;
            return sf_readf_short(file, (short*)dest, length / frame_size) * frame_size;
        }
        
        void rewind()
        {
            sf_seek(file, 0, SEEK_SET);
        }
    };
}
