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
        SInt64 position_;
        
        ALenum format_;
        ALuint sampleRate_;
        UInt32 maxPacketSize_;
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
            
            // Max packet size for streaming
            UInt32 sizeOfProperty = sizeof maxPacketSize_;
            CHECK_OS(AudioFileGetProperty(fileID_, kAudioFilePropertyMaximumPacketSize,
                &sizeOfProperty, &maxPacketSize_));
            
            AudioStreamBasicDescription desc;
            sizeOfProperty = sizeof desc;
            CHECK_OS(AudioFileGetProperty(fileID_, kAudioFilePropertyDataFormat, &sizeOfProperty, &desc));
            
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
            
            // Do we have to swap the endianness for use in OpenAL?
            bigEndian_ = desc.mFormatFlags & kAudioFormatFlagIsBigEndian;
        }
        
    public:
        AudioToolboxFile(const std::wstring& filename)
        {
            ObjRef<NSString> utf8Filename([[NSString alloc] initWithUTF8String: wstringToUTF8(filename).c_str()]);
            ObjRef<NSURL> url([[NSURL alloc] initFileURLWithPath: utf8Filename.get()]);
            #ifdef GOSU_IS_IPHONE
            CHECK_OS(AudioFileOpenURL((CFURLRef)url.get(), kAudioFileReadPermission, 0, &fileID_));
            #else
            // Use FSRef for compatibility with 10.4 Tiger.
            FSRef fsRef;
            CFURLGetFSRef(reinterpret_cast<CFURLRef>(url.get()), &fsRef);
            CHECK_OS(AudioFileOpen(&fsRef, fsRdPerm, 0, &fileID_));
            #endif
            
            init();
        }
        
        AudioToolboxFile(Gosu::Reader reader)
        {
            buffer_.resize(reader.resource().size() - reader.position());
            reader.read(buffer_.data(), buffer_.size());
            
            void* clientData = &buffer_;
            CHECK_OS(AudioFileOpenWithCallbacks(clientData, AudioFile_ReadProc, 0,
                                                AudioFile_GetSizeProc, 0, 0, &fileID_));
            
            init();
        }
        
        ~AudioToolboxFile()
        {
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
            position_ = 0;
        }
        
        std::size_t readData(void* dest, UInt32 length)
        {
            // Class has not been built for both streaming and reading at once :)
            assert(decodedData_.empty());
            
            UInt32 numPackets = length / maxPacketSize_;
            UInt32 numBytes;
            CHECK_OS(AudioFileReadPackets(fileID_, false, &numBytes,
                0, position_, &numPackets, dest));
            position_ += numPackets;
            if (bigEndian_)
                std::transform(static_cast<UInt16*>(dest),
                               static_cast<UInt16*>(dest) + length / 2,
                               static_cast<UInt16*>(dest),
                               htons);
            return numBytes;
        }
    };
}

#endif
