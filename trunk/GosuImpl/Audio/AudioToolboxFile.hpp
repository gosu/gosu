#ifndef GOSU_AUDIO_AudioToolboxFile_MAC_HPP
#define GOSU_AUDIO_AudioToolboxFile_MAC_HPP

#include <AudioToolbox/AudioToolbox.h>
#include <OpenAL/al.h>
#include <Gosu/IO.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <Gosu/Utility.hpp>
#include <boost/utility.hpp>
#include <algorithm>
#include <vector>
#import <Foundation/Foundation.h>

namespace Gosu
{
    class AudioToolboxFile : boost::noncopyable
    {
        AudioFileID fileID;
        SInt64 position;
        mutable std::vector<char> decodedData;
        
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
            CHECK_OS(AudioFileOpenURL((CFURLRef)url.get(), kAudioFileReadPermission, 0, &fileID));
            #else
            // Use FSRef for compatibility with 10.4 Tiger.
            FSRef fsRef;
            CFURLGetFSRef(reinterpret_cast<CFURLRef>(url.get()), &fsRef);
            CHECK_OS(AudioFileOpen(&fsRef, fsRdPerm, 0, &fileID));
            #endif
            position = 0;
        }
        
        AudioToolboxFile(const Gosu::Resource& resource)
        {
            void* clientData = const_cast<Resource*>(&resource);
            CHECK_OS(AudioFileOpenWithCallbacks(clientData, AudioFile_ReadProc, 0,
                                                AudioFile_GetSizeProc, 0, 0, &fileID));
            position = 0;
        }
        
        ~AudioToolboxFile()
        {
            AudioFileClose(fileID);
        }
        
        UInt32 getSizeOfData() const
        {
            UInt64 sizeOfData = 0;
            UInt32 sizeOfProperty = sizeof sizeOfData;
            CHECK_OS(AudioFileGetProperty(fileID, kAudioFilePropertyAudioDataByteCount, &sizeOfProperty, &sizeOfData));
            return sizeOfData;
        }
        
        std::pair<ALuint, ALuint> getFormatAndSampleRate() const
        {
            AudioStreamBasicDescription desc;
            UInt32 sizeOfProperty = sizeof desc;
            CHECK_OS(AudioFileGetProperty(fileID, kAudioFilePropertyDataFormat, &sizeOfProperty, &desc));
            std::pair<ALuint, ALuint> formatAndSampleRate = std::make_pair(0, desc.mSampleRate);
            if (desc.mChannelsPerFrame == 1)
                if (desc.mBitsPerChannel == 8)
                    formatAndSampleRate.first = AL_FORMAT_MONO8;
                else if (desc.mBitsPerChannel == 16)
                    formatAndSampleRate.first = AL_FORMAT_MONO16;
            else if (desc.mChannelsPerFrame == 2)
                if (desc.mBitsPerChannel == 8)
                    formatAndSampleRate.first = AL_FORMAT_STEREO8;
                else if (desc.mBitsPerChannel == 16)
                    formatAndSampleRate.first = AL_FORMAT_STEREO16;
            if (formatAndSampleRate.first == 0)
                throw std::runtime_error("Invalid sample format");
            return formatAndSampleRate;
        }
        
        const std::vector<char>& getDecodedData() const
        {
            // Class has not been built for both streaming and reading at once :)
            assert(position == 0);

            if (decodedData.empty())
            {
                UInt32 sizeOfData = getSizeOfData();
                decodedData.resize(sizeOfData);
                CHECK_OS(AudioFileReadBytes(fileID, false, 0, &sizeOfData, &decodedData[0]));
            }
            return decodedData;
        }
        
        std::size_t readData(void* dest, UInt32 length)
        {
            // Class has not been built for both streaming and reading at once :)
            assert(decodedData.empty());

            CHECK_OS(AudioFileReadBytes(fileID, false, position, &length, dest));
            position += length;
            return length;
        }
    };
}

#endif
