#include "Gosu_FFI_internal.h"

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
    return new Gosu_Channel{sample->sample.play(volume, speed, looping)};
}

GOSU_FFI_API Gosu_Channel* Gosu_Sample_play_pan(Gosu_Sample* sample, double pan, double volume,
                                                double speed, bool looping)
{
    return new Gosu_Channel{sample->sample.play_pan(pan, volume, speed, looping)};
}
