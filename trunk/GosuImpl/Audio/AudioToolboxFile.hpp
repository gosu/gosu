#ifndef GOSU_AUDIO_AUDIOFILE_MAC_HPP
#define GOSU_AUDIO_AUDIOFILE_MAC_HPP

#include <GosuImpl/Audio/AudioFile.hpp>
#include <AudioToolbox/AudioToolbox.h>
#include <OpenAL/al.h>
#include <Gosu/IO.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>
#include <algorithm>
#include <vector>
#include <boost/lexical_cast.hpp>
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
        
        ALenum format_;
        ALuint sampleRate_;
        UInt32 bytesPerFrame_;
        bool bigEndian_;
                
        static OSStatus AudioFile_ReadProc(void* inClientData, SInt64 inPosition, UInt32 requestCount,
                                           void* buffer, UInt32* actualCount)
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
                
        void init()
        {
            // Streaming starts at beginning
            position_ = 0;
            
            AudioStreamBasicDescription desc;
            UInt32 sizeOfProperty = sizeof desc;
            CHECK_OS(ExtAudioFileGetProperty(file_, kExtAudioFileProperty_FileDataFormat,
                &sizeOfProperty, &desc));
            
            // Audio format for OpenAL
            
            if (desc.mFormatFlags & kAudioFormatFlagIsFloat)
                throw std::runtime_error("Float-based formats are unsupported");
            if (desc.mFormatFlags & kAudioFormatFlagIsSignedInteger == 0)
                throw std::runtime_error("Unsigned integer based formats are unsupported");
            if (desc.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
                throw std::runtime_error("Non-interleaved formats are unsupported");
            
            format_ = 0;
            if (desc.mChannelsPerFrame == 1)
                if (desc.mBitsPerChannel == 8)
                    format_ = AL_FORMAT_MONO8;
                else if (desc.mBitsPerChannel == 16)
                    format_ = AL_FORMAT_MONO16;
            else if (desc.mChannelsPerFrame == 2)
                if (desc.mBitsPerChannel == 8)
                    format_ = AL_FORMAT_STEREO8;
                else if (desc.mBitsPerChannel == 16)
                    format_ = AL_FORMAT_STEREO16;
            
            if (desc.mBitsPerChannel == 0)
                format_ = AL_FORMAT_MONO16;
            
            if (format_ == 0)
                throw std::runtime_error("Invalid sample format: " +
                    boost::lexical_cast<std::string>(desc.mChannelsPerFrame) + " channels, " +
                    boost::lexical_cast<std::string>(desc.mBitsPerChannel) + " bits per channel");   
                
            // Sample rate for OpenAL
            sampleRate_ = desc.mSampleRate;
            

            // Client (target) data format

            if (desc.mBytesPerFrame == 0)
            {
                AudioStreamBasicDescription clientData = { 0 };
                clientData.mSampleRate = sampleRate_;
                clientData.mFormatID = kAudioFormatLinearPCM;
                clientData.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
                clientData.mBitsPerChannel = 8 * sizeof(signed int);
                clientData.mChannelsPerFrame = desc.mChannelsPerFrame;
                clientData.mFramesPerPacket = 1;
                clientData.mBytesPerPacket =
                    clientData.mBytesPerFrame = desc.mChannelsPerFrame * sizeof(signed int);
                CHECK_OS(ExtAudioFileSetProperty(file_,
                    kExtAudioFileProperty_ClientDataFormat,
                    sizeof clientData, &clientData));
            }
        }
        
    public:
        AudioToolboxFile(const std::wstring& filename)
        {
            ObjRef<NSString> utf8Filename([[NSString alloc] initWithUTF8String: wstringToUTF8(filename).c_str()]);
            ObjRef<NSURL> url([[NSURL alloc] initFileURLWithPath: utf8Filename.get()]);
            #ifdef GOSU_IS_IPHONE
            CHECK_OS(ExtAudioFileOpenURL((CFURLRef)url.get(), &file_));
            #else
            // Use FSRef for compatibility with 10.4 Tiger.
            FSRef fsRef;
            CFURLGetFSRef(reinterpret_cast<CFURLRef>(url.get()), &fsRef);
            CHECK_OS(ExtAudioFileOpen(&fsRef, &file_));
            #endif
            
            fileID_ = 0;
            
            init();
        }
        
        AudioToolboxFile(Gosu::Reader reader)
        {
            buffer_.resize(reader.resource().size() - reader.position());
            reader.read(buffer_.data(), buffer_.size());
            
            void* clientData = &buffer_;
            CHECK_OS(AudioFileOpenWithCallbacks(clientData, AudioFile_ReadProc, 0,
                                                AudioFile_GetSizeProc, 0, 0, &fileID_));
            CHECK_OS(ExtAudioFileWrapAudioFileID(fileID_, false, &file_));

            init();
        }
        
        ~AudioToolboxFile()
        {
            ExtAudioFileDispose(file_);
        
            if (fileID_)
                AudioFileClose(fileID_);
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
            CHECK_OS(ExtAudioFileSeek(file_, 0));
        }
        
        std::size_t readData(void* dest, UInt32 length)
        {
            AudioBufferList abl;
            abl.mNumberBuffers = 1;
            abl.mBuffers[0].mNumberChannels = 1;
            abl.mBuffers[0].mDataByteSize = length;
            abl.mBuffers[0].mData = dest;
            UInt32 numFrames = 0xffffffff;
            CHECK_OS(ExtAudioFileRead(file_, &numFrames, &abl));
            return abl.mBuffers[0].mDataByteSize;
        }
    };
}

#endif
