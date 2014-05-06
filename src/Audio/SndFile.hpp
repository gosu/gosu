#ifndef GOSUIMPL_AUDIO_SNDFILE_HPP
#define GOSUIMPL_AUDIO_SNDFILE_HPP

#include <Gosu/Audio.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <sndfile.h>

namespace Gosu
{
    class SndFile : public AudioFile
    {
        SNDFILE* file;
        SF_INFO info;
        Reader reader;
        Buffer buffer;
        
        // Cannot use /DELAYLOAD with libsndfile-1.dll because it was compiled
        // using arcane GNU tools of dark magic (or maybe it's the filename).
        #ifdef GOSU_IS_WIN
        static HMODULE dll()
        {
            static HMODULE dll = LoadLibrary(L"libsndfile.dll");
            if (!dll)
                throw std::runtime_error("Cannot find libsndfile.dll");
            return dll;
        }
        
        #define CREATE_STUB(NAME, RETURN, PARAMS, NAMES)                    \
        static RETURN NAME PARAMS                                           \
        {                                                                   \
            typedef RETURN (__cdecl *NAME##_ptr) PARAMS;                            \
            static NAME##_ptr f = (NAME##_ptr)GetProcAddress(dll(), #NAME); \
            if (!f)                                                         \
                throw std::runtime_error("Cannot find " ## #NAME);          \
            return f NAMES;                                                 \
        }
        CREATE_STUB(sf_open_virtual, SNDFILE*,
            (SF_VIRTUAL_IO* sfvirtual, int mode, SF_INFO* sfinfo, void* user_data),
            (sfvirtual, mode, sfinfo, user_data))
        CREATE_STUB(sf_close, int,
            (SNDFILE *sndfile),
            (sndfile))
        CREATE_STUB(sf_read_short, sf_count_t,
            (SNDFILE *sndfile, short *ptr, sf_count_t items),
            (sndfile, ptr, items))
        CREATE_STUB(sf_seek, sf_count_t,
            (SNDFILE *sndfile, sf_count_t frames, int whence),
            (sndfile, frames, whence))
        CREATE_STUB(sf_strerror, const char*,
            (SNDFILE* sndfile),
            (sndfile))
        #endif
        
        static sf_count_t get_filelen(SndFile *self)
        {
            return self->buffer.size();
        }
        
        static sf_count_t seek(sf_count_t offset, int whence, SndFile *self)
        {
            switch (whence)
            {
            case SEEK_SET: self->reader.setPosition(offset); break;
            case SEEK_CUR: self->reader.seek(offset); break;
            case SEEK_END: self->reader.setPosition(self->buffer.size() - offset); break;
            };
            return 0;
        }
        
        static sf_count_t read(void *ptr, sf_count_t count, SndFile *self)
        {
            sf_count_t avail = self->buffer.size() - self->reader.position();
            count = std::min(avail, count);
            self->reader.read(ptr, count);
            return count;
        }
        
        static sf_count_t tell(SndFile *self)
        {
            return self->reader.position();
        }
        
        static SF_VIRTUAL_IO* ioInterface()
        {
            static SF_VIRTUAL_IO io;
            io.get_filelen = (sf_vio_get_filelen)&get_filelen;
            io.seek        = (sf_vio_seek)&seek;
            io.read        = (sf_vio_read)&read;
            io.tell        = (sf_vio_tell)&tell;
            io.write       = NULL;
            return &io;
        }
        
    public:
        SndFile(Reader reader)
        :   file(NULL), reader(buffer.frontReader())
        {
            info.format = 0;
            buffer.resize(reader.resource().size() - reader.position());
            reader.read(buffer.data(), buffer.size());
            file = sf_open_virtual(ioInterface(), SFM_READ, &info, this);
            if (!file)
                throw std::runtime_error(std::string(sf_strerror(NULL)));
        }
        
        SndFile(const std::wstring& filename)
        :   file(NULL), reader(buffer.frontReader())
        {
            info.format = 0;
            #ifdef GOSU_IS_WIN
            loadFile(buffer, filename);
            file = sf_open_virtual(ioInterface(), SFM_READ, &info, this);
            #else
            file = sf_open(wstringToUTF8(filename).c_str(), SFM_READ, &info);
            #endif
            if (!file)
                throw std::runtime_error(std::string(sf_strerror(NULL)));
        }
        
        ~SndFile()
        {
            if (file)
                sf_close(file);
        }
        
        ALenum format() const
        {
            switch (info.channels)
            {
            case 1:
                return AL_FORMAT_MONO16;
            case 2:
                return AL_FORMAT_STEREO16;
            default:
                throw std::runtime_error("Too many channels in audio file");
            };
        }
        
        ALuint sampleRate() const
        {
            return info.samplerate;
        }
        
        std::size_t readData(void* dest, std::size_t length)
        {
            int itemSize = 2 * info.channels;
            return sf_read_short(file, (short*)dest, length / itemSize) * itemSize;
        }
        
        void rewind()
        {
            sf_seek(file, 0, SEEK_SET);
        }
    };
}

#endif
