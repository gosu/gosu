#include <Gosu/Platform.hpp>

// The reason for having this implementation in addition to SDL_sound is that we don't currently
// use SDL at all on iOS. The duplication between this file and SDL_sound_coreaudio.c is pretty
// painful to watch, though. TODO: See if we can use parts of SDL on iOS as well?

#ifdef GOSU_IS_IPHONE

#include "AudioFile.hpp"
#include "AudioImpl.hpp"
#include <Gosu/Utility.hpp>

#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioConverter.h>
#import <AudioToolbox/ExtendedAudioFile.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#include <algorithm>
#include <arpa/inet.h>
#include <stdexcept>
#include <vector>

static void throw_os_error(OSStatus status, unsigned line)
{
    std::string what;
    @autoreleasepool {
        NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
        NSString* message = [NSString stringWithFormat:@"Error 0x%x on line %u: %@", line, status,
                                      error.localizedDescription];
        what = message.UTF8String;
    }
    throw std::runtime_error(what);
}

#define CHECK_OS(status) do { if (status) throw_os_error(status, __LINE__); } while (0)

struct Gosu::AudioFile::Impl
{
    Buffer buffer;
    AudioFileID file_id;
    ExtAudioFileRef file;
    SInt64 position;
    SInt64 seek_offset;
    
    ALenum format;
    ALuint sample_rate;
    UInt32 bytes_per_frame;
    bool big_endian;

    std::vector<char> data;
    
    static OSStatus AudioFile_ReadProc(void* in_client_data, SInt64 in_position,
                                       UInt32 request_count, void* buffer, UInt32* actual_count)
    {
        const Resource& res = *static_cast<Resource*>(in_client_data);
        *actual_count = std::min<UInt32>(request_count,
                                         static_cast<UInt32>(res.size() - in_position));
        res.read(in_position, *actual_count, buffer);
        return noErr;
    }

    static SInt64 AudioFile_GetSizeProc(void* in_client_data)
    {
        const Resource& res = *static_cast<Resource*>(in_client_data);
        return res.size();
    }

    void init_seek_offset()
    {
        AudioConverterRef ac_ref;
        UInt32 acr_size = sizeof ac_ref;
        CHECK_OS(ExtAudioFileGetProperty(file, kExtAudioFileProperty_AudioConverter,
                                         &acr_size, &ac_ref));
        
        AudioConverterPrimeInfo prime_info;
        UInt32 pi_size = sizeof prime_info;
        OSStatus result = AudioConverterGetProperty(ac_ref, kAudioConverterPrimeInfo,
                                                    &pi_size, &prime_info);
        if (result != kAudioConverterErr_PropertyNotSupported) {
            CHECK_OS(result);
            seek_offset = prime_info.leadingFrames;
        }
    }
    
    void init_client_format_based_on(const AudioStreamBasicDescription& base)
    {
        AudioStreamBasicDescription client_data = { 0 };
        sample_rate = client_data.mSampleRate = 22050;
        client_data.mFormatID = kAudioFormatLinearPCM;
        client_data.mFormatFlags = kAudioFormatFlagIsSignedInteger |
                                   kAudioFormatFlagsNativeEndian |
                                   kAudioFormatFlagIsPacked;
        client_data.mBitsPerChannel = 16;
        client_data.mChannelsPerFrame = base.mChannelsPerFrame;
        client_data.mFramesPerPacket = 1;
        client_data.mBytesPerPacket =
            client_data.mBytesPerFrame =
                client_data.mChannelsPerFrame * client_data.mBitsPerChannel / 8;
        CHECK_OS(ExtAudioFileSetProperty(file, kExtAudioFileProperty_ClientDataFormat,
                                         sizeof client_data, &client_data));
        
        init_seek_offset();

        format = client_data.mChannelsPerFrame == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    }
    
    void init()
    {
        // Streaming starts at beginning
        position = 0;
        
        // Unless overridden later, assume that the index into seek() is 0-based
        seek_offset = 0;
        
        AudioStreamBasicDescription desc;
        UInt32 size_of_property = sizeof desc;
        CHECK_OS(ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileDataFormat,
                                         &size_of_property, &desc));

        // Sample rate for OpenAL
        sample_rate = desc.mSampleRate;
        
        // Sanity checks
        if (desc.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
            throw std::runtime_error("Non-interleaved formats are unsupported");
        }
        
        // Easy formats
        format = 0;
        if (desc.mChannelsPerFrame == 1 && desc.mBitsPerChannel == 16) {
            format = AL_FORMAT_MONO16;
        }
        else if (desc.mChannelsPerFrame == 2 && desc.mBitsPerChannel == 16) {
            format = AL_FORMAT_STEREO16;
        }
        
        if (format == 0 ||
                // If format not native for OpenAL, set client data format to enable conversion
                desc.mFormatFlags & kAudioFormatFlagIsBigEndian ||
                desc.mFormatFlags & kAudioFormatFlagIsFloat ||
                !(desc.mFormatFlags & kAudioFormatFlagIsSignedInteger)) {
            init_client_format_based_on(desc);
        }
        else {
            // Just set the old format as the client format so
            // ExtAudioFileSeek will work for us.
            CHECK_OS(ExtAudioFileSetProperty(file, kExtAudioFileProperty_ClientDataFormat,
                                             sizeof desc, &desc));
        }
    }
};

Gosu::AudioFile::AudioFile(const std::string& filename)
: pimpl(new Impl)
{
    NSURL* URL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
    CHECK_OS(ExtAudioFileOpenURL((__bridge CFURLRef) URL, &pimpl->file));
    
    pimpl->file_id = 0;
    
    pimpl->init();
}

Gosu::AudioFile::AudioFile(Reader reader)
: pimpl(new Impl)
{
    pimpl->buffer.resize(reader.resource().size() - reader.position());
    reader.read(pimpl->buffer.data(), pimpl->buffer.size());
    
    void* client_data = &pimpl->buffer;
    CHECK_OS(AudioFileOpenWithCallbacks(client_data, Impl::AudioFile_ReadProc, nullptr,
                                        Impl::AudioFile_GetSizeProc, nullptr, 0, &pimpl->file_id));
    CHECK_OS(ExtAudioFileWrapAudioFileID(pimpl->file_id, false, &pimpl->file));
    
    pimpl->init();
}

Gosu::AudioFile::~AudioFile()
{
    ExtAudioFileDispose(pimpl->file);

    if (pimpl->file_id) {
        AudioFileClose(pimpl->file_id);
    }
}

ALenum Gosu::AudioFile::format() const
{
    return pimpl->format;
}

ALuint Gosu::AudioFile::sample_rate() const
{
    return pimpl->sample_rate;
}

void Gosu::AudioFile::rewind()
{
    CHECK_OS(ExtAudioFileSeek(pimpl->file, 0 + pimpl->seek_offset));
}

std::size_t Gosu::AudioFile::read_data(void* dest, size_t length)
{
    AudioBufferList abl;
    abl.mNumberBuffers = 1;
    abl.mBuffers[0].mNumberChannels = 1;
    abl.mBuffers[0].mDataByteSize = static_cast<UInt32>(length);
    abl.mBuffers[0].mData = dest;
    UInt32 numFrames = 0xffffffff; // give us as many frames as possible given our buffer
    CHECK_OS(ExtAudioFileRead(pimpl->file, &numFrames, &abl));
    return abl.mBuffers[0].mDataByteSize;
}

const std::vector<char>& Gosu::AudioFile::decoded_data()
{
   if (!pimpl->data.empty()) {
       return pimpl->data;
   }

   static const std::size_t INCREMENT = 512 * 1024;

   for (;;) {
       pimpl->data.resize(pimpl->data.size() + INCREMENT);

       auto read_bytes = read_data(&pimpl->data[pimpl->data.size() - INCREMENT], INCREMENT);

       if (read_bytes < INCREMENT) {
           pimpl->data.resize(pimpl->data.size() - INCREMENT + read_bytes);
           break;
       }
   }

   return pimpl->data;
}

#endif
