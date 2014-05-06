#include <Gosu/Platform.hpp>
#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "../MacUtility.hpp"
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <cstdlib>
#include <algorithm>
#include <memory>

namespace Gosu
{
    class ALChannelManagement
    {
        ALChannelManagement(const ALChannelManagement&);
        ALChannelManagement& operator=(const ALChannelManagement&);
        
        static ALCdevice* alDevice;
        static ALCcontext* alContext;

	#ifdef GOSU_IS_IPHONE
        // iOS system limit (possibly not for newer devices)
        enum { NUM_SOURCES = 32 };
	#else
        enum { NUM_SOURCES = 255 };
	#endif
        static ALuint alSources[NUM_SOURCES];
        static ALuint currentToken;
        static ALuint currentTokens[NUM_SOURCES];
        
    public:
        enum { NO_TOKEN = -1, NO_SOURCE = -1, NO_FREE_CHANNEL = -1 };
        
        static ALCdevice* device()
        {
            return alDevice;
        }
        
        static ALCcontext* context()
        {
            return alContext;
        }
        
        ALChannelManagement()
        {
            // Open preferred device
            alDevice = alcOpenDevice(0);
            alContext = alcCreateContext(alDevice, 0);
            alcMakeContextCurrent(alContext);
            alGenSources(NUM_SOURCES, alSources);
            std::fill(currentTokens, currentTokens + NUM_SOURCES,
                static_cast<ALuint>(NO_TOKEN));
        }
        
        ~ALChannelManagement()
        {
            alDeleteSources(NUM_SOURCES, alSources);
            alcMakeContextCurrent(0);
            alcDestroyContext(alContext);
            alcCloseDevice(alDevice);
        }
        
        std::pair<int, int> reserveChannel()
        {
            int i;
            for (i = 1; i <= NUM_SOURCES; ++i)
            {
                if (i == NUM_SOURCES)
                    return std::make_pair<int, int>(NO_FREE_CHANNEL, NO_TOKEN);
                if (currentTokens[i] == NO_TOKEN)
                    break;
                ALint state;
                alGetSourcei(alSources[i], AL_SOURCE_STATE, &state);
                if (state != AL_PLAYING && state != AL_PAUSED)
                    break;
            }
            ++currentToken;
            currentTokens[i] = currentToken;
            return std::make_pair(i, int(currentToken));
        }
        
        int sourceIfStillPlaying(int channel, int token) const
        {
            if (channel != NO_FREE_CHANNEL && currentTokens[channel] == token)
                return alSources[channel];
            return NO_SOURCE;
        }
        
        int sourceForSongs() const
        {
            return alSources[0];
        }
    };
    ALCdevice* ALChannelManagement::alDevice = 0;
    ALCcontext* ALChannelManagement::alContext = 0;
    ALuint ALChannelManagement::alSources[NUM_SOURCES];
    ALuint ALChannelManagement::currentToken = 0;
    ALuint ALChannelManagement::currentTokens[NUM_SOURCES];

    GOSU_UNIQUE_PTR<ALChannelManagement> alChannelManagement;
    
    void releaseAllOpenALResources()
    {
        alChannelManagement.reset();
    }
    
    ALCcontext *sharedContext()
    {
        return ALChannelManagement::context();
    }
}
