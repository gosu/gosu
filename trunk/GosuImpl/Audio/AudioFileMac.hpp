#ifndef GOSU_AUDIO_AUDIOFILE_MAC_HPP
#define GOSU_AUDIO_AUDIOFILE_MAC_HPP

#include <AudioToolbox/AudioToolbox.h>
#include <OpenAL/al.h>
#include <GosuImpl/MacUtility.hpp>
#include <Gosu/Utility.hpp>
#include <boost/utility.hpp>
#include <vector>
#import <Foundation/Foundation.h>

namespace Gosu
{
    class AudioFile : boost::noncopyable
    {
        AudioFileID fileID;
        mutable std::vector<char> decodedData;
        
    public:
        AudioFile(const std::wstring& filename)
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
        }
        
        ~AudioFile()
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
            if (decodedData.empty())
            {
                UInt32 sizeOfData = getSizeOfData();
                decodedData.resize(sizeOfData);
                CHECK_OS(AudioFileReadBytes(fileID, false, 0, &sizeOfData, &decodedData[0]));
            }
            return decodedData;
        }
    };
}

#endif
