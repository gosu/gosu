#include <Gosu/Audio.hpp>
#include "Gosu_Song.h"

extern "C" {
  Gosu_Song* Gosu_Song_create(const char* filename)
  {
    return reinterpret_cast<Gosu_Song*>( new Gosu::Song(filename) );
  }

  void Gosu_Song_destroy(Gosu_Song* song)
  {
    delete( reinterpret_cast<Gosu::Song*>( song ));
  }

  void Gosu_Song_play(Gosu_Song* song, bool looping)
  {
    reinterpret_cast<Gosu::Song*>( song )->play(looping);
  }

  bool Gosu_Song_playing(Gosu_Song* song)
  {
    return reinterpret_cast<Gosu::Song*>( song )->playing();
  }

  void Gosu_Song_pause(Gosu_Song* song)
  {
    reinterpret_cast<Gosu::Song*>( song )->pause();
  }

  bool Gosu_Song_paused(Gosu_Song* song)
  {
    return reinterpret_cast<Gosu::Song*>( song )->paused();
  }

  void Gosu_Song_stop(Gosu_Song* song)
  {
    reinterpret_cast<Gosu::Song*>( song )->stop();
  }

  double Gosu_Song_volume(Gosu_Song* song){
    return reinterpret_cast<Gosu::Song*>( song )->volume();
  }

  void Gosu_Song_set_volume(Gosu_Song* song, double volume){
    return reinterpret_cast<Gosu::Song*>( song )->set_volume(volume);
  }

  Gosu_Song* Gosu_Song_current_song()
  {
    return reinterpret_cast<Gosu_Song*>(Gosu::Song::current_song());
  }
}