#include "Gosu_Sample.h"
#include <Gosu/Audio.hpp>

struct Gosu_Sample
{
    Gosu::Sample sample;
};

GOSU_FFI_API Gosu_Sample* Gosu_Sample_create(const char* filename)
{
    return new Gosu_Sample{Gosu::Sample{filename}};
}

GOSU_FFI_API void Gosu_Sample_destroy(Gosu_Sample* sample)
{
    delete sample;
}

GOSU_FFI_API Gosu_Channel* Gosu_Sample_play(Gosu_Sample* sample, double volume, double speed,
                                            bool looping)
{
    Gosu::Channel channel = sample->sample.play(volume, speed, looping);
    return reinterpret_cast<Gosu_Channel*>(new Gosu::Channel{channel});
}

GOSU_FFI_API Gosu_Channel* Gosu_Sample_play_pan(Gosu_Sample* sample, double pan, double volume,
                                                double speed, bool looping)
{
    Gosu::Channel channel = sample->sample.play_pan(pan, volume, speed, looping);
    return reinterpret_cast<Gosu_Channel*>(new Gosu::Channel{channel});
}
