#pragma once

#import "AudioFile.hpp"
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioConverter.h>
#import <AudioToolbox/ExtendedAudioFile.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <Gosu/IO.hpp>
#import <Gosu/Platform.hpp>
#import <Gosu/Utility.hpp>
#import <OpenAL/al.h>
#import <algorithm>
#import <arpa/inet.h>
#import <stdexcept>
#import <vector>

inline static void throw_os_error(OSStatus status, unsigned line)
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

namespace Gosu
{
    class AudioToolboxFile : public AudioFile
    {
        Gosu::Buffer buffer_;
        AudioFileID file_id_;
        ExtAudioFileRef file_;
        SInt64 position_;
        SInt64 seek_offset_;
        
        ALenum format_;
        ALuint sample_rate_;
        UInt32 bytes_per_frame_;
        bool big_endian_;
        
        static OSStatus AudioFile_ReadProc(void* in_client_data, SInt64 in_position,
            UInt32 request_count, void* buffer, UInt32* actual_count)
        {
            const Resource& res = *static_cast<Resource*>(in_client_data);
            *actual_count = std::min<UInt32>(request_count, res.size() - in_position);
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
            CHECK_OS(ExtAudioFileGetProperty(file_, kExtAudioFileProperty_AudioConverter,
                                             &acr_size, &ac_ref));
            
            AudioConverterPrimeInfo prime_info;
            UInt32 pi_size = sizeof prime_info;
            OSStatus result = AudioConverterGetProperty(ac_ref, kAudioConverterPrimeInfo,
                                                        &pi_size, &prime_info);
            if (result != kAudioConverterErr_PropertyNotSupported) {
                CHECK_OS(result);
                seek_offset_ = prime_info.leadingFrames;
            }
        }
        
        void init_client_format_based_on(const AudioStreamBasicDescription& base)
        {
            AudioStreamBasicDescription client_data = { 0 };
            sample_rate_ = client_data.mSampleRate = 22050;
            client_data.mFormatID = kAudioFormatLinearPCM;
            client_data.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            client_data.mBitsPerChannel = 16;
            client_data.mChannelsPerFrame = base.mChannelsPerFrame;
            client_data.mFramesPerPacket = 1;
            client_data.mBytesPerPacket =
                client_data.mBytesPerFrame =
                    client_data.mChannelsPerFrame * client_data.mBitsPerChannel / 8;
            CHECK_OS(ExtAudioFileSetProperty(file_, kExtAudioFileProperty_ClientDataFormat,
                                             sizeof client_data, &client_data));
            
            init_seek_offset();

            format_ = client_data.mChannelsPerFrame == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        }
        
        void init()
        {
            // Streaming starts at beginning
            position_ = 0;
            
            // Unless overridden later, assume that the index into seek() is 0-based
            seek_offset_ = 0;
            
            AudioStreamBasicDescription desc;
            UInt32 size_of_property = sizeof desc;
            CHECK_OS(ExtAudioFileGetProperty(file_, kExtAudioFileProperty_FileDataFormat,
                                             &size_of_property, &desc));

            // Sample rate for OpenAL
            sample_rate_ = desc.mSampleRate;
            
            // Sanity checks
            if (desc.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
                throw std::runtime_error("Non-interleaved formats are unsupported");
            }
            
            // Easy formats
            format_ = 0;
            if (desc.mChannelsPerFrame == 1) {
                /*if (desc.mBitsPerChannel == 8)
                    format_ = AL_FORMAT_MONO8;
                else*/ if (desc.mBitsPerChannel == 16)
                    format_ = AL_FORMAT_MONO16;
            }
            else if (desc.mChannelsPerFrame == 2) {
                /*if (desc.mBitsPerChannel == 8)
                    format_ = AL_FORMAT_STEREO8;
                else */if (desc.mBitsPerChannel == 16)
                    format_ = AL_FORMAT_STEREO16;
            }
            
            if (format_ == 0 ||
                    // If format not native for OpenAL, set client data format
                    // to enable conversion
                    desc.mFormatFlags & kAudioFormatFlagIsBigEndian ||
                    desc.mFormatFlags & kAudioFormatFlagIsFloat ||
                    !(desc.mFormatFlags & kAudioFormatFlagIsSignedInteger)) {
                init_client_format_based_on(desc);
            }
            else {
                // Just set the old format as the client format so
                // ExtAudioFileSeek will work for us.
                CHECK_OS(ExtAudioFileSetProperty(file_, kExtAudioFileProperty_ClientDataFormat,
                                                 sizeof desc, &desc));
            }
        }
        
    public:
        AudioToolboxFile(const std::string& filename)
        {
            NSURL* URL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]];
            CHECK_OS(ExtAudioFileOpenURL((__bridge CFURLRef) URL, &file_));
            
            file_id_ = 0;
            
            init();
        }
        
        AudioToolboxFile(Gosu::Reader reader)
        {
            buffer_.resize(reader.resource().size() - reader.position());
            reader.read(buffer_.data(), buffer_.size());
            
            // TODO: This fails on iOS with MP3 files.
            // TODO: ^ Is the comment above still true on non-ancient iOS versions?
            
            void* client_data = &buffer_;
            CHECK_OS(AudioFileOpenWithCallbacks(client_data, AudioFile_ReadProc, 0,
                                                AudioFile_GetSizeProc, 0, 0, &file_id_));
            CHECK_OS(ExtAudioFileWrapAudioFileID(file_id_, false, &file_));
            
            init();
        }
        
        ~AudioToolboxFile()
        {
            ExtAudioFileDispose(file_);

            if (file_id_) {
                AudioFileClose(file_id_);
            }
        }
        
        ALenum format() const
        {
            return format_;
        }
        
        ALuint sample_rate() const
        {
            return sample_rate_;
        }
        
        void rewind()
        {
            CHECK_OS(ExtAudioFileSeek(file_, 0 + seek_offset_));
        }
        
        std::size_t read_data(void* dest, size_t length)
        {
            AudioBufferList abl;
            abl.mNumberBuffers = 1;
            abl.mBuffers[0].mNumberChannels = 1;
            abl.mBuffers[0].mDataByteSize = length;
            abl.mBuffers[0].mData = dest;
            UInt32 numFrames = 0xffffffff; // give us as many frames as possible given our buffer
            CHECK_OS(ExtAudioFileRead(file_, &numFrames, &abl));
            return abl.mBuffers[0].mDataByteSize;
        }
    };
}
