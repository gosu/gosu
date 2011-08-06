#include <Gosu/Platform.hpp>
#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <GosuImpl/MacUtility.hpp>
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

        enum { NUM_SOURCES = 32 }; // This is what the iPhone supports, I hear.
        static ALuint alSources[NUM_SOURCES];
        static ALuint currentToken;
        static ALuint currentTokens[NUM_SOURCES];
        
    public:
        enum { NO_TOKEN = -1, NO_SOURCE = -1, NO_FREE_CHANNEL = -1 };
        
        static ALCdevice* device()
        {
            return alDevice;
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
            return std::make_pair<int, int>(i, currentToken);
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

    std::auto_ptr<ALChannelManagement> alChannelManagement;
}
