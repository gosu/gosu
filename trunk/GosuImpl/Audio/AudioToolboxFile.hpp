#ifndef GOSU_AUDIO_AUDIOFILE_MAC_HPP
#define GOSU_AUDIO_AUDIOFILE_MAC_HPP

#include <GosuImpl/Audio/AudioFile.hpp>
#include <AudioToolbox/AudioToolbox.h>
#include <OpenAL/al.h>
#include <Gosu/IO.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <Gosu/Utility.hpp>
#include <algorithm>
#include <vector>
#include <boost/lexical_cast.hpp>
#import <Foundation/Foundation.h>

namespace Gosu
{
    class AudioToolboxFile : public AudioFile
    {
        Gosu::Buffer buffer_;
        AudioFileID fileID_;
        SInt64 position_;
        mutable std::vector<char> decodedData_;
        
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
            position_ = 0;
        }
        
        AudioToolboxFile(Gosu::Reader reader)
        {
            buffer_.resize(reader.resource().size() - reader.position());
            reader.read(buffer_.data(), buffer_.size());
            
            void* clientData = &buffer_;
            CHECK_OS(AudioFileOpenWithCallbacks(clientData, AudioFile_ReadProc, 0,
                                                AudioFile_GetSizeProc, 0, 0, &fileID_));
            position_ = 0;
        }
        
        ~AudioToolboxFile()
        {
            AudioFileClose(fileID_);
        }
        
        UInt32 sizeOfData() const
        {
            UInt64 size = 0;
            UInt32 sizeOfProperty = sizeof size;
            CHECK_OS(AudioFileGetProperty(fileID_, kAudioFilePropertyAudioDataByteCount, &sizeOfProperty, &size));
            return size;
        }
        
        ALenum format() const
        {
            AudioStreamBasicDescription desc;
            UInt32 sizeOfProperty = sizeof desc;
            CHECK_OS(AudioFileGetProperty(fileID_, kAudioFilePropertyDataFormat, &sizeOfProperty, &desc));

            if (desc.mChannelsPerFrame == 1)
                if (desc.mBitsPerChannel == 8)
                    return AL_FORMAT_MONO8;
                else if (desc.mBitsPerChannel == 16)
                    return AL_FORMAT_MONO16;
            else if (desc.mChannelsPerFrame == 2)
                if (desc.mBitsPerChannel == 8)
                    return AL_FORMAT_STEREO8;
                else if (desc.mBitsPerChannel == 16)
                    return AL_FORMAT_STEREO16;
            
            if (desc.mBitsPerChannel == 0)
                return AL_FORMAT_STEREO16; // Let's just guess for streaming files!
            
            throw std::runtime_error("Invalid sample format: " +
                boost::lexical_cast<std::string>(desc.mChannelsPerFrame) + " channels, " +
                boost::lexical_cast<std::string>(desc.mBitsPerChannel) + " bits per channel");
        }
        
        ALuint sampleRate() const
        {
            AudioStreamBasicDescription desc;
            UInt32 sizeOfProperty = sizeof desc;
            CHECK_OS(AudioFileGetProperty(fileID_, kAudioFilePropertyDataFormat, &sizeOfProperty, &desc));
            return desc.mSampleRate;
        }
        
        void rewind()
        {
            position_ = 0;
        }
        
        const std::vector<char>& decodedData()
        {
            // Class has not been built for both streaming and reading at once :)
            assert(position_ == 0);

            if (decodedData_.empty())
            {
                UInt32 size = sizeOfData();
                decodedData_.resize(size);
                CHECK_OS(AudioFileReadBytes(fileID_, false, 0, &size, &decodedData_[0]));
            }
            return decodedData_;
        }
        
        std::size_t readData(void* dest, UInt32 length)
        {
            // Class has not been built for both streaming and reading at once :)
            assert(decodedData_.empty());
            
            /*UInt32 possiblePackets = 1;
            
            UInt32 numBytes;
            AudioStreamPacketDescription packetDescriptions[possiblePackets];
            CHECK_OS(AudioFileReadPackets(fileID_, false,
                &numBytes, &packedDescriptions[0], position_, 
   UInt32                       *outNumBytes,
   AudioStreamPacketDescription *outPacketDescriptions,
   SInt64                       inStartingPacket,
   UInt32                       *ioNumPackets,
   void                         *outBuffer)*/
            
            CHECK_OS(AudioFileReadBytes(fileID_, false, position_, &length, dest));
            position_ += length;
            return length;
        }
    };
}

#endif
