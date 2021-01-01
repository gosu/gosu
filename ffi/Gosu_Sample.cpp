#include <Gosu/Audio.hpp>
#include "Gosu_Sample.h"

extern "C" {

Gosu_Sample *Gosu_Sample_create(const char *filename)
{
  return reinterpret_cast<Gosu_Sample *>( new Gosu::Sample(filename) );
}

void Gosu_Sample_destroy(Gosu_Sample *sample)
{
  delete( reinterpret_cast<Gosu::Sample *>( sample ) );
}

Gosu_Channel *Gosu_Sample_play(Gosu_Sample *sample, double volume, double speed, bool looping)
{
  Gosu::Channel channel = reinterpret_cast<Gosu::Sample *>( sample )->play(volume, speed, looping);

  return reinterpret_cast<Gosu_Channel *>( new Gosu::Channel(channel) );
}

Gosu_Channel *Gosu_Sample_play_pan(Gosu_Sample *sample, double pan, double volume, double speed, bool looping)
{
  Gosu::Channel channel = reinterpret_cast<Gosu::Sample *>( sample )->play_pan(pan, volume, speed, looping);

  return reinterpret_cast<Gosu_Channel *>( new Gosu::Channel(channel) );
}

}