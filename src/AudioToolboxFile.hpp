#ifndef GOSU_SRC_AUDIOTOOLBOXFILE_HPP
#define GOSU_SRC_AUDIOTOOLBOXFILE_HPP

#include "AudioFile.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <OpenAL/al.h>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>
#include "AppleUtility.hpp"
#include <algorithm>
#include <vector>
#include <arpa/inet.h>
#import <Foundation/Foundation.h>

namespace Gosu
{
    class AudioToolboxFile : public AudioFile
    {
        Gosu::Buffer buffer_;
        AudioFileID fileID_;
        ExtAudioFileRef file_;
        SInt64 position_;
        SInt64 seekOffset_;
        
        ALenum format_;
        ALuint sampleRate_;
        UInt32 bytesPerFrame_;
        bool bigEndian_;
        
        static OSStatus AudioFile_ReadProc(void* inClientData, SInt64 inPosition,
            UInt32 requestCount, void* buffer, UInt32* actualCount)
        {
            const Resource& res = *static_cast<Resource*>(inClientData);
            *actualCount = std::min<UInt32>(requestCount, res.size() - inPosition);
            res.read(inPosition, *actualCount, buffer);
            return noErr;
        }
        
        static SInt64 AudioFile_GetSizeProc(void* inClientData)
        {
            const Resource& res = *static_cast<Resource*>(inClientData);
            return res.size();
        }
        
        void initSeekOffset()
        {
            AudioConverterRef acRef;
            UInt32 acrSize = sizeof acRef;
            CHECK_OS(ExtAudioFileGetProperty(file_, kExtAudioFileProperty_AudioConverter,
                &acrSize, &acRef));
            
            AudioConverterPrimeInfo primeInfo;
            UInt32 piSize = sizeof primeInfo;
            OSStatus result = AudioConverterGetProperty(acRef, kAudioConverterPrimeInfo,
                &piSize, &primeInfo);
            if (result != kAudioConverterErr_PropertyNotSupported)
            {
                CHECK_OS(result);
                seekOffset_ = primeInfo.leadingFrames;
            }
        }
        
        void initClientFormatBasedOn(const AudioStreamBasicDescription& base)
        {
            AudioStreamBasicDescription clientData = { 0 };
            sampleRate_ = clientData.mSampleRate = 22050;
            clientData.mFormatID = kAudioFormatLinearPCM;
            clientData.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            clientData.mBitsPerChannel = 16;
            clientData.mChannelsPerFrame = base.mChannelsPerFrame;
            clientData.mFramesPerPacket = 1;
            clientData.mBytesPerPacket =
                clientData.mBytesPerFrame =
                    clientData.mChannelsPerFrame * clientData.mBitsPerChannel / 8;
            CHECK_OS(ExtAudioFileSetProperty(file_,
                kExtAudioFileProperty_ClientDataFormat,
                sizeof clientData, &clientData));
            
            initSeekOffset();
                
            format_ = clientData.mChannelsPerFrame == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        }
        
        void init()
        {
            // Streaming starts at beginning
            position_ = 0;
            
            // Unless overridden later, assume that the index into seek() is 0-based
            seekOffset_ = 0;
            
            AudioStreamBasicDescription desc;
            UInt32 sizeOfProperty = sizeof desc;
            CHECK_OS(ExtAudioFileGetProperty(file_, kExtAudioFileProperty_FileDataFormat,
                &sizeOfProperty, &desc));

            // Sample rate for OpenAL
            sampleRate_ = desc.mSampleRate;
            
            // Sanity checks
            if (desc.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
                throw std::runtime_error("Non-interleaved formats are unsupported");
            
            // Easy formats
            format_ = 0;
            if (desc.mChannelsPerFrame == 1)
            {
                /*if (desc.mBitsPerChannel == 8)
                    format_ = AL_FORMAT_MONO8;
                else*/ if (desc.mBitsPerChannel == 16)
                    format_ = AL_FORMAT_MONO16;
            }
            else if (desc.mChannelsPerFrame == 2)
            {
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
                initClientFormatBasedOn(desc);
            }
            else {
                // Just set the old format as the client format so
                // ExtAudioFileSeek will work for us.
                CHECK_OS(ExtAudioFileSetProperty(file_,
                    kExtAudioFileProperty_ClientDataFormat,
                    sizeof desc, &desc));
            }
        }
        
    public:
        AudioToolboxFile(const std::wstring& filename)
        {
            NSString *utf8Filename = [NSString stringWithUTF8String:wstringToUTF8(filename).c_str()];
            NSURL *URL = [NSURL fileURLWithPath:utf8Filename];
            CHECK_OS(ExtAudioFileOpenURL((__bridge CFURLRef)URL, &file_));
            
            fileID_ = 0;
            
            init();
        }
        
        AudioToolboxFile(Gosu::Reader reader)
        {
            buffer_.resize(reader.resource().size() - reader.position());
            reader.read(buffer_.data(), buffer_.size());
            
            // TODO: This fails on iOS with MP3 files.
            // TODO: ^ Is the comment above still true on non-ancient iOS versions?
            
            void* clientData = &buffer_;
            CHECK_OS(AudioFileOpenWithCallbacks(clientData, AudioFile_ReadProc, 0,
                                                AudioFile_GetSizeProc, 0, 0, &fileID_));
            CHECK_OS(ExtAudioFileWrapAudioFileID(fileID_, false, &file_));
            
            init();
        }
        
        ~AudioToolboxFile()
        {
            ExtAudioFileDispose(file_);
        
            if (fileID_) {
                AudioFileClose(fileID_);
            }
        }
        
        ALenum format() const
        {
            return format_;
        }
        
        ALuint sampleRate() const
        {
            return sampleRate_;
        }
        
        void rewind()
        {
            CHECK_OS(ExtAudioFileSeek(file_, 0 + seekOffset_));
        }
        
        std::size_t readData(void* dest, size_t length)
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

#endif
