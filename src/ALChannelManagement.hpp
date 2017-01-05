#include <Gosu/Platform.hpp>
#include <cstdlib>
#include <algorithm>
#include <memory>
#ifdef GOSU_IS_MAC
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace Gosu
{
    class ALChannelManagement
    {
        ALChannelManagement(const ALChannelManagement&) = delete;
        ALChannelManagement& operator=(const ALChannelManagement&) = delete;
        ALChannelManagement(ALChannelManagement&&) = delete;
        ALChannelManagement& operator=(ALChannelManagement&&) = delete;
        
        static ALCdevice* al_device;
        static ALCcontext* al_context;

        #ifdef GOSU_IS_IPHONE
        // iOS system limit.
        enum { NUM_SOURCES = 32 };
        #else
        enum { NUM_SOURCES = 255 };
        #endif
        static ALuint al_sources[NUM_SOURCES];
        static ALuint current_token;
        static ALuint current_tokens[NUM_SOURCES];
        
    public:
        enum { NO_TOKEN = -1, NO_SOURCE = -1, NO_FREE_CHANNEL = -1 };
        
        static ALCdevice* device()
        {
            return al_device;
        }
        
        static ALCcontext* context()
        {
            return al_context;
        }
        
        ALChannelManagement()
        {
            // Open preferred device
            al_device = alcOpenDevice(0);
            al_context = alcCreateContext(al_device, 0);
            alcMakeContextCurrent(al_context);
            alGenSources(NUM_SOURCES, al_sources);
            std::fill(current_tokens, current_tokens + NUM_SOURCES, static_cast<ALuint>(NO_TOKEN));
        }
        
        ~ALChannelManagement()
        {
            alDeleteSources(NUM_SOURCES, al_sources);
            alcMakeContextCurrent(0);
            alcDestroyContext(al_context);
            alcCloseDevice(al_device);
        }
        
        std::pair<int, int> reserve_channel()
        {
            int i;
            for (i = 1; i <= NUM_SOURCES; ++i) {
                if (i == NUM_SOURCES) {
                    return std::make_pair<int, int>(NO_FREE_CHANNEL, NO_TOKEN);
                }
                if (current_tokens[i] == NO_TOKEN) {
                    break;
                }
                ALint state;
                alGetSourcei(al_sources[i], AL_SOURCE_STATE, &state);
                if (state != AL_PLAYING && state != AL_PAUSED) {
                    break;
                }
            }
            ++current_token;
            current_tokens[i] = current_token;
            return std::make_pair(i, int(current_token));
        }
        
        int source_if_still_playing(int channel, int token) const
        {
            if (channel != NO_FREE_CHANNEL && current_tokens[channel] == token) {
                return al_sources[channel];
            }
            else {
                return NO_SOURCE;
            }
        }
        
        int source_for_songs() const
        {
            return al_sources[0];
        }
    };
    ALCdevice* ALChannelManagement::al_device = 0;
    ALCcontext* ALChannelManagement::al_context = 0;
    ALuint ALChannelManagement::al_sources[NUM_SOURCES];
    ALuint ALChannelManagement::current_token = 0;
    ALuint ALChannelManagement::current_tokens[NUM_SOURCES];

    std::unique_ptr<ALChannelManagement> al_channel_management;
    
    void release_all_openal_resources()
    {
        al_channel_management.reset();
    }
    
    ALCcontext *shared_openal_context()
    {
        return ALChannelManagement::context();
    }
}
